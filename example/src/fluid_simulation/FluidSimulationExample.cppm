module;

#include "imgui.h"

export module examples.fluid_simulation;

import std;

import examples.fluid_simulation.particle;

import shark;

using namespace shark;

export {
class FluidSimulationExample : public Layer {
public:
    static constexpr float   PARTICLE_RADIUS = 0.05f;
    static constexpr float   GRAVITY         = 9.8f;
    static constexpr Vector3 BOUNDS          = {4.0f, 4.0f, 4.0f};
    static constexpr float   DAMPING         = 0.9f;

    enum class SpawnPreset : std::uint8_t {
        Waves,
        CenteredBlock,
    };

    void onAttach() override;
    void onDetach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

    void resetSimulation();
    void resetBox();

private:
    SpawnPreset currentPreset = SpawnPreset::Waves;
    void        renderImGui();

    Scene            scene;
    OrbitCamera*     camera         = nullptr;
    InstancedEntity* particleGroup  = nullptr;
    Entity*          boxFrameEntity = nullptr;

    // Box tilt dynamics and force state
    Vector3 boxRot{0.0f, 0.0f, 0.0f};    // in degrees (pitch X, yaw Y, roll Z)
    Vector3 boxAngVel{0.0f, 0.0f, 0.0f}; // in deg/s
    Vector3 boxAngAcc{0.0f, 0.0f, 0.0f};
    Vector3 prevBoxAngVel{0.0f, 0.0f, 0.0f};
    Vector3 boxTargetRot{0.0f, 0.0f, 0.0f};

    bool   isDraggingBox   = false;
    double lastMouseX      = 0.0;
    double lastMouseY      = 0.0;
    float  tiltSensitivity = 0.25f;

    int   currentParticleCount = 10000;
    int   uiParticleCount      = 10000;
    bool  isPaused             = false;
    float gravity              = 9.8f;
    float restitution          = 0.25f;
    float viscosity            = 0.018f;
    float relaxation           = 0.40f;
    float friction             = 0.0f;

    std::vector<Particle> particles;
    SpatialHashGrid       grid;
};

} // export

namespace {
inline float randomFloat() {
    static thread_local std::mt19937                          gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(gen);
}

inline Vector4 speedToColor(float speed, float maxSpeed = 8.0f) {
    float t = std::clamp(speed / maxSpeed, 0.0f, 1.0f);
    if (t < 0.5f) {
        float factor = t * 2.0f;
        return {0.1f, 0.25f + (0.65f * factor), 0.90f + (0.05f * factor), 1.0f};
    }
    float factor = (t - 0.5f) * 2.0f;
    return {0.10f + (0.90f * factor), 0.90f + (0.05f * factor), 0.95f - (0.45f * factor), 1.0f};
}
} // namespace

void FluidSimulationExample::resetBox() {
    boxRot        = {0.0f, 0.0f, 0.0f};
    boxAngVel     = {0.0f, 0.0f, 0.0f};
    boxAngAcc     = {0.0f, 0.0f, 0.0f};
    prevBoxAngVel = {0.0f, 0.0f, 0.0f};
    boxTargetRot  = {0.0f, 0.0f, 0.0f};
    isDraggingBox = false;

    if (boxFrameEntity != nullptr) {
        boxFrameEntity->setPosition({0.0f, 0.0f, 0.0f})->setRotation(boxRot);
    }
}

void FluidSimulationExample::resetSimulation() {
    resetBox();

    auto& instanceData = particleGroup->getInstancesMutable();
    particles.resize(static_cast<std::size_t>(currentParticleCount));
    instanceData.resize(static_cast<std::size_t>(currentParticleCount));

    const float spacing = PARTICLE_RADIUS * 2.15f;
    const float halfZ   = (BOUNDS.z * 0.5f) - (PARTICLE_RADIUS * 1.5f);
    const float floorY  = -(BOUNDS.y * 0.5f) + (PARTICLE_RADIUS * 1.5f);

    if (currentPreset == SpawnPreset::Waves) {
        const float maxDepthX = BOUNDS.x * 0.85f;
        int         nx        = std::max(1, static_cast<int>(std::floor(maxDepthX / spacing)));
        int         remaining = (currentParticleCount + nx - 1) / nx;
        int         nz = std::max(1, static_cast<int>(std::round(std::sqrt(static_cast<float>(remaining) / 1.6f))));
        nz             = std::min(nz, std::max(1, static_cast<int>((BOUNDS.z * 0.40f) / spacing)));
        int ny         = (remaining + nz - 1) / nz;

        const float startZ  = halfZ;
        const float startY  = floorY;
        const float offsetX = (static_cast<float>(nx - 1) * 0.5f) * spacing;

        int particleIdx = 0;
        for (int iy = 0; iy < ny && particleIdx < currentParticleCount; ++iy) {
            for (int iz = 0; iz < nz && particleIdx < currentParticleCount; ++iz) {
                for (int ix = 0; ix < nx && particleIdx < currentParticleCount; ++ix) {
                    float x = (static_cast<float>(ix) * spacing) - offsetX;
                    float y = startY + (static_cast<float>(iy) * spacing);
                    float z = startZ - (static_cast<float>(iz) * spacing);

                    float jitterX = (randomFloat() - 0.5f) * (PARTICLE_RADIUS * 0.08f);
                    float jitterZ = (randomFloat() - 0.5f) * (PARTICLE_RADIUS * 0.08f);

                    particles[particleIdx].position     = {.x = x + jitterX, .y = y, .z = z + jitterZ};
                    particles[particleIdx].prevPosition = particles[particleIdx].position;
                    particles[particleIdx].velocity     = {.x = 0.0f, .y = 0.0f, .z = 0.0f};

                    instanceData[particleIdx].modelMatrix = Matrix4::translate(particles[particleIdx].position);
                    instanceData[particleIdx].color       = {0.1f, 0.35f, 0.90f, 1.0f};
                    particleIdx++;
                }
            }
        }
    } else { // CenteredBlock
        int   side   = static_cast<int>(std::round(std::cbrt(currentParticleCount)));
        float offset = static_cast<float>(side - 1) * 0.5f;

        for (int i = 0; i < currentParticleCount; i++) {
            int ix = i % side;
            int iy = (i / side) % side;
            int iz = i / (side * side);

            float x = (static_cast<float>(ix) - offset) * spacing;
            float y = ((static_cast<float>(iy) - offset) * spacing) + 0.6f;
            float z = (static_cast<float>(iz) - offset) * spacing;

            float jitterX = (randomFloat() - 0.5f) * (PARTICLE_RADIUS * 0.08f);
            float jitterZ = (randomFloat() - 0.5f) * (PARTICLE_RADIUS * 0.08f);

            particles[i].position     = {.x = x + jitterX, .y = y, .z = z + jitterZ};
            particles[i].prevPosition = particles[i].position;
            particles[i].velocity     = {.x = 0.0f, .y = 0.0f, .z = 0.0f};

            instanceData[i].modelMatrix = Matrix4::translate(particles[i].position);
            instanceData[i].color       = {0.1f, 0.35f, 0.90f, 1.0f};
        }
    }
}

void FluidSimulationExample::onAttach() {
    getApp()->setScene(&scene);

    camera = scene.add<OrbitCamera>()->setDistance(10.0f);
    camera->setRotateButton(MouseButton::Right);
    camera->setPanEnabled(false);

    auto sharedSphereMesh = std::make_shared<Mesh>(
        MeshFactory::sphere(6, Vector3{.x = 0.0f, .y = 0.0f, .z = 0.0f}).scale(PARTICLE_RADIUS)
    );

    particleGroup = scene.add<InstancedEntity>()->setMesh(sharedSphereMesh);

    auto boxMesh   = std::make_shared<Mesh>(MeshFactory::boxFrame(BOUNDS, 0.035f, Vector3{0.8f, 0.9f, 1.0f}));
    boxFrameEntity = scene.add<Entity>()->setMesh(boxMesh)->setColor({0.85f, 0.92f, 1.0f, 1.0f});

    resetBox();
    resetSimulation();
    grid.init(BOUNDS, PARTICLE_RADIUS * 2.0f);
}

void FluidSimulationExample::onDetach() {
    if ((getApp() != nullptr) && getApp()->getScene() == &scene) {
        getApp()->setScene(nullptr);
    }
    particles.clear();
    particleGroup  = nullptr;
    boxFrameEntity = nullptr;
    camera         = nullptr;
    scene.clear();
}

void FluidSimulationExample::onUpdate(float deltaTime) {
    float dt = std::min(deltaTime, 0.02f);
    if (dt <= 0.0f) {
        return;
    }

    // Process interactive mouse dragging of the box
    const bool imguiWantsMouse = Application::isMouseCaptured();
    double     mouseX          = getApp()->getCursorX();
    double     mouseY          = getApp()->getCursorY();

    if (getApp()->isMouseButtonPressed(MouseButton::Left)) {
        if (!isDraggingBox) {
            if (!imguiWantsMouse) {
                isDraggingBox = true;
                lastMouseX    = mouseX;
                lastMouseY    = mouseY;
            }
        } else {
            auto dx = static_cast<float>(mouseX - lastMouseX);
            auto dy = static_cast<float>(mouseY - lastMouseY);

            float yawRad = (camera != nullptr) ? (camera->getRotation().y * std::numbers::pi_v<float> / 180.0f) : 0.0f;

            // Camera right vector in XZ plane for horizontal tilt axis (pitch)
            Vector3 camRight{std::sin(yawRad), 0.0f, -std::cos(yawRad)};

            // dy tilts pitch around camera horizontal axis; dx yaws around vertical Y axis
            Vector3 deltaRot = (camRight * (dy * tiltSensitivity)) + Vector3{0.0f, dx * tiltSensitivity, 0.0f};

            boxTargetRot += deltaRot;

            constexpr float MAX_TILT = 60.0f;
            boxTargetRot             = clamp(
                boxTargetRot, Vector3{-MAX_TILT, -MAX_TILT, -MAX_TILT}, Vector3{MAX_TILT, MAX_TILT, MAX_TILT}
            );

            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }
    } else {
        isDraggingBox = false;
        boxTargetRot  = {0.0f, 0.0f, 0.0f};
    }

    // Physics step for box tilt (spring-damper dynamics)
    constexpr float springStiffness = 45.0f;
    constexpr float springDamping   = 8.0f;

    Vector3 rotError     = boxTargetRot - boxRot;
    Vector3 rotSpringAcc = (rotError * springStiffness) - (boxAngVel * springDamping);
    boxAngVel += rotSpringAcc * dt;
    boxRot += boxAngVel * dt;

    const float invDt = 1.0f / dt;
    boxAngAcc         = (boxAngVel - prevBoxAngVel) * invDt;

    // Safety clamp on acceleration spikes from sudden mouse jumps
    constexpr float MAX_ANG_ACC = 400.0f;
    if (boxAngAcc.lengthSq() > (MAX_ANG_ACC * MAX_ANG_ACC)) {
        boxAngAcc *= MAX_ANG_ACC / boxAngAcc.length();
    }

    prevBoxAngVel = boxAngVel;

    // Build rotation matrix (Z * Y * X)
    Matrix4 rotMat = Matrix4::trs({0.0f, 0.0f, 0.0f}, boxRot, {1.0f, 1.0f, 1.0f});

    // Compute effective accelerations in box-local frame
    Vector3 localGravity = rotMat.transformVectorTransposed(Vector3{0.0f, -gravity, 0.0f});

    Vector3 radAngVel = rotMat.transformVectorTransposed(boxAngVel * (std::numbers::pi_v<float> / 180.0f));
    Vector3 radAngAcc = rotMat.transformVectorTransposed(boxAngAcc * (std::numbers::pi_v<float> / 180.0f));

    if (!isPaused) {
        // Predict particle positions via forces
        shark::exec::for_each(shark::exec::par_unseq, particles, [localGravity, radAngVel, radAngAcc, dt](Particle& p) {
            Vector3 r      = p.position; // local position relative to box center
            Vector3 a_ang  = -cross(radAngAcc, r);
            Vector3 a_cent = -cross(radAngVel, cross(radAngVel, r));
            Vector3 a_cor  = cross(radAngVel, p.velocity) * (-2.0f);

            Vector3 effAcc = localGravity + a_ang + a_cent + a_cor;

            p.velocity += effAcc * dt;
            p.prevPosition = p.position;
            p.position += p.velocity * dt;
        });

        // Build spatial grid with predicted positions
        grid.build(particles);

        // Iteratively project collisions (PBD relaxation)
        constexpr int solverIterations = 3;
        for (auto _ : std::views::iota(0, solverIterations)) {
            grid.solveCollisions(particles, PARTICLE_RADIUS * 2.0f, relaxation);
            shark::exec::for_each(shark::exec::par_unseq, particles, [](Particle& p) {
                p.clampToBounds(BOUNDS, PARTICLE_RADIUS);
            });
        }

        constexpr float MAX_SPEED    = 15.0f;
        constexpr float MAX_SPEED_SQ = MAX_SPEED * MAX_SPEED;

        const float rest = restitution;
        const float fric = friction;
        shark::exec::for_each(shark::exec::par_unseq, particles, [invDt, dt, rest, fric](Particle& p) {
            p.velocity = (p.position - p.prevPosition) * invDt;

            float speedSq = p.velocity.lengthSq();
            if (speedSq > MAX_SPEED_SQ) {
                float factor = MAX_SPEED / std::sqrt(speedSq);
                p.velocity *= factor;
            }

            p.resolveBoundaryVelocity(BOUNDS, PARTICLE_RADIUS, dt, rest, fric);
        });

        // Viscosity smoothing (XSPH)
        grid.applyViscosity(particles, PARTICLE_RADIUS * 2.5f, viscosity);
    }

    // Update visual transforms
    if (boxFrameEntity != nullptr) {
        boxFrameEntity->setPosition({0.0f, 0.0f, 0.0f})->setRotation(boxRot);
    }

    auto& instanceData = particleGroup->getInstancesMutable();
    shark::exec::for_each_index(
        shark::exec::par_unseq, particles.size(), [this, &rotMat, &instanceData](std::size_t i) {
            Vector3 worldPos            = rotMat.transformVector(particles[i].position);
            instanceData[i].modelMatrix = Matrix4::translate(worldPos);
            instanceData[i].color       = speedToColor(particles[i].velocity.length());
        }
    );
}

void FluidSimulationExample::renderImGui() {
    ImGui::SetNextWindowPos(ImVec2(10.0f, 28.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(340, 320), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls");

    // Particle count controls (applies on slider release)
    if (ImGui::SliderInt("Particles", &uiParticleCount, 500, 50000)) {
        uiParticleCount = std::clamp(uiParticleCount, 100, 100000);
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && uiParticleCount != currentParticleCount) {
        currentParticleCount = uiParticleCount;
        resetSimulation();
    }

    ImGui::Separator();
    if (ImGui::Button(isPaused ? " Resume " : " Pause ")) {
        isPaused = !isPaused;
    }

    ImGui::SameLine();
    if (ImGui::RadioButton("Waves", currentPreset == SpawnPreset::Waves)) {
        currentPreset = SpawnPreset::Waves;
        resetSimulation();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Cube", currentPreset == SpawnPreset::CenteredBlock)) {
        currentPreset = SpawnPreset::CenteredBlock;
        resetSimulation();
    }

    ImGui::Separator();
    ImGui::SliderFloat("Restitution", &restitution, 0.0f, 0.6f, "%.2f");
    ImGui::SliderFloat("Friction", &friction, 0.0f, 0.1f, "%.4f");
    ImGui::SliderFloat("Viscosity", &viscosity, 0.0f, 0.05f, "%.4f");
    ImGui::SliderFloat("Relaxation", &relaxation, 0.1f, 0.8f, "%.2f");
    ImGui::SliderFloat("Gravity", &gravity, 0.0f, 25.0f, "%.1f");

    ImGui::End();
}

void FluidSimulationExample::onRender(Renderer& renderer) {
    renderer.render(scene, getApp()->getWidth(), getApp()->getHeight());
    renderImGui();
}
