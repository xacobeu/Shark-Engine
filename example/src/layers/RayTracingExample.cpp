#include "RayTracingExample.hpp"

#include <algorithm>
#include <cmath>
#include <print>
#include <span>

#include "imgui.h"

#include "Application.hpp"
#include "renderer/geometry/Vertex.hpp"

void RayTracingExample::onAttach() {
    std::println("[DEBUG] GPU Raytracer layer attached");

    raytraceShader =
        getApp()->getDevice()->createShader("res/shaders/raytrace.vert.glsl", "res/shaders/raytrace.frag.glsl");

    // Quad geometry (NDC covering [-1, 1])
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
}

void RayTracingExample::onUpdate(float deltaTime) {
    time += deltaTime;

    // Dynamic sphere motion
    sphere1Pos.x = std::sin(time) * 1.5f;
    sphere1Pos.y = std::cos(time * 1.5f) * 0.5f;
}

void RayTracingExample::onRender(Renderer&) {
    raytraceShader->bind();

    auto safeHeight   = static_cast<float>(getApp()->getHeight());
    safeHeight        = std::max(safeHeight, 1.0f);
    float aspectRatio = static_cast<float>(getApp()->getWidth()) / safeHeight;
    raytraceShader->setFloat("u_AspectRatio", aspectRatio);
    raytraceShader->setFloat("u_Time", time);
    raytraceShader->setVec3("u_CameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
    raytraceShader->setVec3("u_SpherePos", sphere1Pos.x, sphere1Pos.y, sphere1Pos.z);
    raytraceShader->setFloat("u_SphereRadius", sphere1Radius);
    raytraceShader->setInt("u_EnableShadows", enableShadows ? 1 : 0);
    raytraceShader->setInt("u_EnableReflections", enableReflections ? 1 : 0);
    raytraceShader->setFloat("u_ReflectionStrength", reflectionStrength);

    quadVbo->bind();
    getApp()->getDevice()->drawArrays(6);
    quadVbo->unbind();
    raytraceShader->unbind();

    ImGui::SetNextWindowSize(ImVec2(350, 360), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
    ImGui::Begin("GPU Raytracer Settings");

    ImGui::Text("FPS: %.1f", static_cast<double>(ImGui::GetIO().Framerate));
    ImGui::Checkbox("Enable Shadows", &enableShadows);
    ImGui::Checkbox("Enable Reflections", &enableReflections);
    if (enableReflections) {
        ImGui::SliderFloat("Reflection Strength", &reflectionStrength, 0.0f, 1.0f);
    }

    ImGui::Separator();
    ImGui::Text("Camera Settings");
    ImGui::SliderFloat3("Camera Position", &cameraPos.x, -5.0f, 5.0f);

    ImGui::Separator();
    ImGui::Text("Red Sphere Settings");
    ImGui::SliderFloat3("Sphere Center", &sphere1Pos.x, -5.0f, 5.0f);
    ImGui::SliderFloat("Sphere Radius", &sphere1Radius, 0.1f, 3.0f);

    ImGui::End();
}
