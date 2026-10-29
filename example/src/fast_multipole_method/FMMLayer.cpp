#include "FMMLayer.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <functional>
#include <numbers>
#include <random>
#include <span>

#include "imgui.h"

namespace {
// matplotlib's "magma" colormap, sampled at 17 evenly spaced stops (t = i / 16).
constexpr std::array<Vector3, 17> MAGMA_STOPS = {
    Vector3{0.0015f, 0.0005f, 0.0139f}, Vector3{0.0396f, 0.0311f, 0.1335f}, Vector3{0.1131f, 0.0655f, 0.2768f},
    Vector3{0.2117f, 0.0620f, 0.4186f}, Vector3{0.3167f, 0.0717f, 0.4854f}, Vector3{0.4147f, 0.1104f, 0.5047f},
    Vector3{0.5128f, 0.1482f, 0.5076f}, Vector3{0.6136f, 0.1818f, 0.4985f}, Vector3{0.7164f, 0.2150f, 0.4753f},
    Vector3{0.8169f, 0.2559f, 0.4365f}, Vector3{0.9043f, 0.3196f, 0.3881f}, Vector3{0.9609f, 0.4183f, 0.3596f},
    Vector3{0.9867f, 0.5356f, 0.3822f}, Vector3{0.9961f, 0.6537f, 0.4462f}, Vector3{0.9969f, 0.7696f, 0.5349f},
    Vector3{0.9924f, 0.8843f, 0.6401f}, Vector3{0.9871f, 0.9914f, 0.7495f},
};

Vector4 magma(float t) {
    const float   scaled = std::clamp(t, 0.0f, 1.0f) * static_cast<float>(MAGMA_STOPS.size() - 1);
    const auto    index  = std::min(static_cast<std::size_t>(scaled), MAGMA_STOPS.size() - 2);
    const float   f      = scaled - static_cast<float>(index);
    const Vector3 c      = (MAGMA_STOPS[index] * (1.0f - f)) + (MAGMA_STOPS[index + 1] * f);
    return {c.x, c.y, c.z, 1.0f};
}
} // namespace

void FMMLayer::onAttach() {
    getApp()->setScene(&scene);

    auto* camera = scene.add<OrbitCamera>()->setDistance(140.0f)->setPanEnabled(false);
    camera->setRotation(Vector3{40.0f, 0.0f, 0.0f});

    auto sphereMesh = std::make_shared<Mesh>(MeshFactory::sphere(6, Vector3{1.0f, 1.0f, 1.0f}));
    particleGroup   = scene.add<InstancedEntity>()->setMesh(sphereMesh);

    fmm.setConfig(fmmConfig);
    resetSimulation(currentParticleCount);
}

void FMMLayer::onDetach() {
    particles.clear();
    instanceData.clear();
    particleGroup = nullptr;
}

void FMMLayer::resetSimulation(std::size_t count) {
    currentParticleCount = count;
    uiParticleCount      = static_cast<int>(count);

    particles.resize(currentParticleCount);
    instanceData.resize(currentParticleCount);

    if (distribution == Distribution::Disc) {
        generateKuzminDisk(particles, instanceData, {});
    } else {
        generateUniformDistribution(particles, instanceData, {});
    }

    colorMaxSpeed = measureColorMaxSpeed();
    for (std::size_t i = 0; i < currentParticleCount; ++i) {
        instanceData[i].color = magma(particles[i].velocity.length() / colorMaxSpeed);
    }

    if (particleGroup != nullptr) {
        particleGroup->setInstances(instanceData);
    }

    solve();
    observedErrorPct = fmm.computeObservedError(particles, G_CONSTANT, ERROR_SAMPLE_COUNT);
    frameCount       = 0;
}

void FMMLayer::solve() {
    fmm.buildTree(particles, {0.0f, 0.0f, 0.0f}, BOUNDS.x * 0.5f);
    fmm.execute(particles, G_CONSTANT);
}

float FMMLayer::measureColorMaxSpeed() const {
    if (particles.empty()) {
        return 1.0f;
    }

    // Speed scales with sqrt(total mass), so a fixed range saturates at high particle counts.
    // Anchor the top of the colormap to a multiple of the RMS speed instead.
    const float sumSq = shark::exec::transform_reduce_index(
        shark::exec::par_unseq, particles.size(), 0.0f, std::plus<>{},
        [this](std::size_t i) { return particles[i].velocity.lengthSq(); }
    );
    const float rms = std::sqrt(sumSq / static_cast<float>(particles.size()));
    return std::max(rms * COLOR_RMS_MULTIPLIER, 1e-3f);
}

void FMMLayer::onUpdate(float deltaTime) {
    if (isPaused) {
        return;
    }

    const float dt = std::clamp(deltaTime, 0.001f, 0.02f);

    const auto startTime = std::chrono::steady_clock::now();
    solve();
    lastFmmTimeMs = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - startTime).count();

    if (++frameCount >= ERROR_UPDATE_INTERVAL_FRAMES) {
        observedErrorPct = fmm.computeObservedError(particles, G_CONSTANT, ERROR_SAMPLE_COUNT);
        frameCount       = 0;
    }

    // Ease the color range toward the current speed distribution so it adapts without flickering
    const float smoothing = 1.0f - std::exp(-dt / COLOR_RANGE_SMOOTHING_SECONDS);
    colorMaxSpeed += (measureColorMaxSpeed() - colorMaxSpeed) * smoothing;

    const float maxSpeed = colorMaxSpeed;
    shark::exec::for_each_index(shark::exec::par_unseq, particles.size(), [this, dt, maxSpeed](std::size_t i) {
        auto& p = particles[i];
        p.integrate(dt);
        instanceData[i].modelMatrix = Matrix4::translateScale(p.position, PARTICLE_RADIUS);
        instanceData[i].color       = magma(p.velocity.length() / maxSpeed);
    });

    if (particleGroup != nullptr) {
        particleGroup->setInstances(instanceData);
    }
}

void FMMLayer::renderImGui() {
    ImGui::SetNextWindowPos(ImVec2(10.0f, 28.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(340, 340), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls");

    ImGui::SliderInt("Particles", &uiParticleCount, 500, 100000);
    if (ImGui::IsItemDeactivatedAfterEdit() && uiParticleCount != static_cast<int>(currentParticleCount)) {
        resetSimulation(static_cast<std::size_t>(uiParticleCount));
    }

    ImGui::Separator();
    if (ImGui::Button(isPaused ? " Resume " : " Pause ")) {
        isPaused = !isPaused;
    }
    ImGui::SameLine();
    if (ImGui::Button(" Reset ")) {
        resetSimulation(currentParticleCount);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Disc", distribution == Distribution::Disc)) {
        distribution = Distribution::Disc;
        resetSimulation(currentParticleCount);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Uniform", distribution == Distribution::Uniform)) {
        distribution = Distribution::Uniform;
        resetSimulation(currentParticleCount);
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.40f, 0.85f, 1.0f, 1.0f), "Observed Error: %.2f %%", observedErrorPct);
    ImGui::Spacing();

    bool changed = false;
    changed |= ImGui::SliderFloat("Opening Angle", &fmmConfig.theta, 0.30f, 1.30f, "%.2f");

    int nCritInt = static_cast<int>(fmmConfig.nCrit);
    if (ImGui::SliderInt("Leaf Capacity", &nCritInt, 8, 128)) {
        fmmConfig.nCrit = static_cast<std::size_t>(nCritInt);
        changed         = true;
    }

    changed |= ImGui::SliderFloat("Softening", &fmmConfig.softening, 0.10f, 1.50f, "%.2f");

    if (changed) {
        fmm.setConfig(fmmConfig);
        if (isPaused && !particles.empty()) {
            solve();
            observedErrorPct = fmm.computeObservedError(particles, G_CONSTANT, ERROR_SAMPLE_COUNT);
        } else {
            frameCount = ERROR_UPDATE_INTERVAL_FRAMES;
        }
    }

    ImGui::Separator();
    ImGui::Text("Octree Nodes: %zu", fmm.getNodeCount());
    ImGui::Text("FMM Solve: %.2f ms", lastFmmTimeMs);

    ImGui::End();
}

void FMMLayer::onRender(Renderer& renderer) {
    renderer.render(scene, getApp()->getWidth(), getApp()->getHeight());
    renderImGui();
}

void FMMLayer::generateKuzminDisk(
    std::span<fmm::Particle> particles, std::span<InstanceData> instanceData, const KuzminDiskConfig& cfg
) {
    const std::size_t count = std::min(particles.size(), instanceData.size());
    if (count == 0) {
        return;
    }

    std::mt19937                          gen(cfg.seed);
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
    std::normal_distribution<float>       distY(0.0f, cfg.verticalDispersion);

    const float maxRadius = BOUNDS.x * cfg.maxRadiusRatio;
    const float totalMass = static_cast<float>(count) * cfg.particleMass;
    const float a2        = cfg.coreRadius * cfg.coreRadius;

    for (std::size_t i = 0; i < count; ++i) {
        // Invert the Kuzmin disk's cumulative mass fraction to sample a radius
        const float u = dist01(gen);
        const float r = std::min(cfg.coreRadius * std::sqrt((1.0f / (1.0f - (u * 0.92f))) - 1.0f), maxRadius);

        const float theta = dist01(gen) * 2.0f * std::numbers::pi_v<float>;
        const float y     = std::clamp(distY(gen), -cfg.verticalDispersion * 4.0f, cfg.verticalDispersion * 4.0f);

        particles[i].position     = {r * std::cos(theta), y, r * std::sin(theta)};
        particles[i].mass         = cfg.particleMass;
        particles[i].acceleration = {0.0f, 0.0f, 0.0f};

        // Circular orbit speed: v = sqrt(G * M * r^2 / (r^2 + a^2)^1.5)
        const float r2         = r * r;
        const float vCirc      = std::sqrt((G_CONSTANT * totalMass * r2) / std::pow(r2 + a2, 1.5f));
        const float dispersion = (dist01(gen) - 0.5f) * (vCirc * cfg.thermalDispersion);

        const Vector3 tangent = {-std::sin(theta), 0.0f, std::cos(theta)};
        particles[i].velocity = tangent * (vCirc + dispersion);

        instanceData[i].modelMatrix = Matrix4::translateScale(particles[i].position, PARTICLE_RADIUS);
    }
}

void FMMLayer::generateUniformDistribution(
    std::span<fmm::Particle> particles, std::span<InstanceData> instanceData, const UniformConfig& cfg
) {
    const std::size_t count = std::min(particles.size(), instanceData.size());
    if (count == 0) {
        return;
    }

    std::mt19937                          gen(cfg.seed);
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
    std::uniform_real_distribution<float> distCos(-1.0f, 1.0f);

    const float maxRadius = BOUNDS.x * cfg.maxRadiusRatio;

    for (std::size_t i = 0; i < count; ++i) {
        const float r        = maxRadius * std::cbrt(dist01(gen));
        const float cosTheta = distCos(gen);
        const float sinTheta = std::sqrt(std::max(0.0f, 1.0f - (cosTheta * cosTheta)));
        const float phi      = dist01(gen) * 2.0f * std::numbers::pi_v<float>;

        particles[i].position     = {r * sinTheta * std::cos(phi), r * cosTheta, r * sinTheta * std::sin(phi)};
        particles[i].mass         = cfg.particleMass;
        particles[i].acceleration = {0.0f, 0.0f, 0.0f};
        particles[i].velocity     = {0.0f, 0.0f, 0.0f};

        instanceData[i].modelMatrix = Matrix4::translateScale(particles[i].position, PARTICLE_RADIUS);
    }
}
