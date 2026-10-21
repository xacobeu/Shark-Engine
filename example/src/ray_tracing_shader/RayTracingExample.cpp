#include "RayTracingExample.hpp"

#include <algorithm>
#include <cmath>
#include <print>
#include <random>
#include <span>

#include "imgui.h"

#include "Application.hpp"
#include "renderer/geometry/Vertex.hpp"

namespace {
Vector3 hsvToRgb(float h, float s, float v) {
    float c = v * s;
    float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    if (h < 60.0f) {
        r = c;
        g = x;
    } else if (h < 120.0f) {
        r = x;
        g = c;
    } else if (h < 180.0f) {
        g = c;
        b = x;
    } else if (h < 240.0f) {
        g = x;
        b = c;
    } else if (h < 300.0f) {
        r = x;
        b = c;
    } else {
        r = c;
        b = x;
    }
    return Vector3{.x = r + m, .y = g + m, .z = b + m};
}
} // namespace

void RayTracingExample::initSpheres() {
    spheres.clear();
    spheres.reserve(MAX_SPHERES);

    SphereInstance heroSphere;
    heroSphere.basePos    = Vector3{.x = 0.0f, .y = 0.0f, .z = -4.5f};
    heroSphere.currentPos = heroSphere.basePos;
    heroSphere.radius     = 0.95f;
    heroSphere.color      = Vector3{.x = 1.0f, .y = 0.0f, .z = 0.0f};
    heroSphere.shininess  = 0.0f;
    heroSphere.animSpeed  = 1.2f;
    heroSphere.animPhase  = 0.0f;
    heroSphere.animHeight = 0.5f;
    spheres.push_back(heroSphere);

    std::random_device                    rd;
    std::mt19937                          rng(rd());
    std::uniform_real_distribution<float> distX(-6.0f, 6.0f);
    std::uniform_real_distribution<float> distZ(-12.0f, -2.2f);
    std::uniform_real_distribution<float> distRadius(0.25f, 0.65f);
    std::uniform_real_distribution<float> distHue(0.0f, 360.0f);
    std::uniform_real_distribution<float> distSaturation(0.70f, 1.0f);
    std::uniform_real_distribution<float> distValue(0.80f, 1.0f);
    std::uniform_real_distribution<float> distShininess(0.60f, 0.98f);
    std::uniform_real_distribution<float> distSpeed(0.8f, 2.5f);
    std::uniform_real_distribution<float> distPhase(0.0f, 6.2831853f);
    std::uniform_real_distribution<float> distHeight(0.2f, 0.55f);
    std::uniform_real_distribution<float> distMirror(0.0f, 1.0f);

    for (int i = 1; i < MAX_SPHERES; ++i) {
        SphereInstance sphere;
        sphere.radius = distRadius(rng);

        float posX = distX(rng);
        float posZ = distZ(rng);
        float posY = -1.0f + sphere.radius + (distHeight(rng) * 0.8f);

        sphere.basePos    = Vector3{.x = posX, .y = posY, .z = posZ};
        sphere.currentPos = sphere.basePos;

        if (distMirror(rng) < 0.15f) {
            sphere.color     = Vector3{.x = 0.95f, .y = 0.95f, .z = 0.98f};
            sphere.shininess = 0.98f;
        } else {
            sphere.color     = hsvToRgb(distHue(rng), distSaturation(rng), distValue(rng));
            sphere.shininess = distShininess(rng);
        }

        sphere.animSpeed  = distSpeed(rng);
        sphere.animPhase  = distPhase(rng);
        sphere.animHeight = distHeight(rng);

        spheres.push_back(sphere);
    }
}

void RayTracingExample::onAttach() {
    std::println("[DEBUG] GPU Raytracer layer attached");
    getApp()->setScene(nullptr);

    raytraceShader =
        getApp()->getDevice()->createShader("res/shaders/raytrace.vert.glsl", "res/shaders/raytrace.frag.glsl");

    std::array<Vertex, 6> quadVertices = {
        Vertex{
            .position = Vector3{.x = -1.0f, .y = 1.0f, .z = 0.0f},
            .normal   = Vector3{.x = 0.0f, .y = 0.0f, .z = 1.0f},
            .uv       = Vector2{.x = 0.0f, .y = 1.0f},
            .color    = Vector3{.x = 1.0f, .y = 1.0f, .z = 1.0f},
        },
        Vertex{
            .position = Vector3{.x = -1.0f, .y = -1.0f, .z = 0.0f},
            .normal   = Vector3{.x = 0.0f, .y = 0.0f, .z = 1.0f},
            .uv       = Vector2{.x = 0.0f, .y = 0.0f},
            .color    = Vector3{.x = 1.0f, .y = 1.0f, .z = 1.0f},
        },
        Vertex{
            .position = Vector3{.x = 1.0f, .y = -1.0f, .z = 0.0f},
            .normal   = Vector3{.x = 0.0f, .y = 0.0f, .z = 1.0f},
            .uv       = Vector2{.x = 1.0f, .y = 0.0f},
            .color    = Vector3{.x = 1.0f, .y = 1.0f, .z = 1.0f},
        },

        Vertex{
            .position = Vector3{.x = -1.0f, .y = 1.0f, .z = 0.0f},
            .normal   = Vector3{.x = 0.0f, .y = 0.0f, .z = 1.0f},
            .uv       = Vector2{.x = 0.0f, .y = 1.0f},
            .color    = Vector3{.x = 1.0f, .y = 1.0f, .z = 1.0f},
        },
        Vertex{
            .position = Vector3{.x = 1.0f, .y = -1.0f, .z = 0.0f},
            .normal   = Vector3{.x = 0.0f, .y = 0.0f, .z = 1.0f},
            .uv       = Vector2{.x = 1.0f, .y = 0.0f},
            .color    = Vector3{.x = 1.0f, .y = 1.0f, .z = 1.0f},
        },
        Vertex{
            .position = Vector3{.x = 1.0f, .y = 1.0f, .z = 0.0f},
            .normal   = Vector3{.x = 0.0f, .y = 0.0f, .z = 1.0f},
            .uv       = Vector2{.x = 1.0f, .y = 1.0f},
            .color    = Vector3{.x = 1.0f, .y = 1.0f, .z = 1.0f},
        },
    };

    quadVbo = getApp()->getDevice()->createVertexBuffer(std::span{quadVertices});

    initSpheres();
}

void RayTracingExample::onDetach() {
    raytraceShader.reset();
    quadVbo.reset();
}

void RayTracingExample::onUpdate(float deltaTime) {
    time += deltaTime;

    if (spheres.empty()) {
        return;
    }

    // Hero sphere dynamic movement
    spheres[0].currentPos.x = spheres[0].basePos.x + std::sin(time * spheres[0].animSpeed) * 1.6f;
    spheres[0].currentPos.y =
        spheres[0].basePos.y + std::cos(time * spheres[0].animSpeed * 1.3f) * spheres[0].animHeight;
    spheres[0].currentPos.z = spheres[0].basePos.z;

    // Remaining shiny spheres floating and bobbing animation
    if (animateSpheres) {
        for (std::size_t i = 1; i < spheres.size(); ++i) {
            float phase             = (time * spheres[i].animSpeed * animSpeedMultiplier) + spheres[i].animPhase;
            spheres[i].currentPos.x = spheres[i].basePos.x;
            spheres[i].currentPos.y = spheres[i].basePos.y + (std::sin(phase) * spheres[i].animHeight);
            spheres[i].currentPos.z = spheres[i].basePos.z;
        }
    }
}

void RayTracingExample::onRender(Renderer&) {
    if (!raytraceShader || !quadVbo) {
        return;
    }

    raytraceShader->bind();

    auto safeHeight   = static_cast<float>(getApp()->getHeight());
    safeHeight        = std::max(safeHeight, 1.0f);
    float aspectRatio = static_cast<float>(getApp()->getWidth()) / safeHeight;
    raytraceShader->setFloat("u_AspectRatio", aspectRatio);
    raytraceShader->setFloat("u_Time", time);
    raytraceShader->setVec3("u_CameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
    raytraceShader->setInt("u_EnableShadows", enableShadows ? 1 : 0);
    raytraceShader->setInt("u_EnableReflections", enableReflections ? 1 : 0);
    raytraceShader->setFloat("u_ReflectionStrength", reflectionStrength);

    int count = std::clamp(sphereCount, 1, static_cast<int>(spheres.size()));
    raytraceShader->setInt("u_SphereCount", count);

    std::array<float, MAX_SPHERES * 4> posRadData{};
    std::array<float, MAX_SPHERES * 4> colShinyData{};
    for (int i = 0; i < count; ++i) {
        posRadData[(i * 4) + 0] = spheres[i].currentPos.x;
        posRadData[(i * 4) + 1] = spheres[i].currentPos.y;
        posRadData[(i * 4) + 2] = spheres[i].currentPos.z;
        posRadData[(i * 4) + 3] = spheres[i].radius;

        colShinyData[(i * 4) + 0] = spheres[i].color.x;
        colShinyData[(i * 4) + 1] = spheres[i].color.y;
        colShinyData[(i * 4) + 2] = spheres[i].color.z;
        colShinyData[(i * 4) + 3] = spheres[i].shininess;
    }

    raytraceShader->setVec4Array("u_SpherePosRad[0]", posRadData.data(), static_cast<std::size_t>(count));
    raytraceShader->setVec4Array("u_SphereColShiny[0]", colShinyData.data(), static_cast<std::size_t>(count));

    quadVbo->bind();
    getApp()->getDevice()->drawArrays(6);
    quadVbo->unbind();
    raytraceShader->unbind();

    ImGui::SetNextWindowPos(ImVec2(10.0f, 28.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(360, 480), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls");

    ImGui::Text("Shiny Balls Settings");
    ImGui::SliderInt("Ball Count", &sphereCount, 1, MAX_SPHERES);
    if (ImGui::Button(" Re-roll Random Balls ")) {
        initSpheres();
    }
    ImGui::Checkbox("Animate Balls", &animateSpheres);
    if (animateSpheres) {
        ImGui::SliderFloat("Animation Speed", &animSpeedMultiplier, 0.0f, 3.0f);
    }

    ImGui::Separator();
    ImGui::Text("Rendering Settings");
    ImGui::Checkbox("Enable Shadows", &enableShadows);
    ImGui::Checkbox("Enable Reflections", &enableReflections);
    if (enableReflections) {
        ImGui::SliderFloat("Reflection Strength", &reflectionStrength, 0.0f, 1.0f);
    }

    ImGui::Separator();
    ImGui::Text("Camera Settings");
    ImGui::SliderFloat3("Camera Position", &cameraPos.x, -5.0f, 5.0f);

    if (!spheres.empty()) {
        ImGui::Separator();
        ImGui::Text("Hero Ball (Red) Settings");
        ImGui::SliderFloat3("Sphere Center", &spheres[0].basePos.x, -5.0f, 5.0f);
        ImGui::SliderFloat("Sphere Radius", &spheres[0].radius, 0.1f, 3.0f);
        ImGui::ColorEdit3("Ball Color", &spheres[0].color.x);
        ImGui::SliderFloat("Shininess", &spheres[0].shininess, 0.0f, 1.0f);
    }

    ImGui::End();
}
