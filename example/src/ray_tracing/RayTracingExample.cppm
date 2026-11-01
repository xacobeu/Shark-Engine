module;

#include "imgui.h"

export module examples.ray_tracing;

import std;

import shark;

using namespace shark;

export {
class RayTracingExample : public Layer {
public:
    static constexpr int MAX_SPHERES = 128;

    struct SphereInstance {
        Vector3 basePos;
        Vector3 currentPos;
        float   radius     = 0.5f;
        Vector3 color      = {1.0f, 1.0f, 1.0f};
        float   shininess  = 0.8f;
        float   animSpeed  = 1.0f;
        float   animPhase  = 0.0f;
        float   animHeight = 0.4f;
    };

    void onAttach() override;
    void onDetach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

    void initSpheres();

private:
    // std140 mirrors of the shader's uniform blocks
    struct Params {
        Vector4                     cameraPos;
        float                       aspectRatio        = 1.0f;
        float                       reflectionStrength = 0.0f;
        std::int32_t                sphereCount        = 0;
        std::int32_t                enableShadows      = 0;
        std::int32_t                enableReflections  = 0;
        std::array<std::int32_t, 3> padding{};
    };
    static_assert(sizeof(Params) == 48);

    struct GpuSphere {
        Vector4 posRad;
        Vector4 colShiny;
    };
    using SphereBlock = std::array<GpuSphere, MAX_SPHERES>;

    std::shared_ptr<IPipeline> raytracePipeline;
    std::shared_ptr<IBuffer>   quadVbo;

    float time               = 0.0f;
    bool  enableShadows      = true;
    bool  enableReflections  = true;
    float reflectionStrength = 0.65f;

    int   sphereCount         = 10;
    bool  animateSpheres      = true;
    float animSpeedMultiplier = 1.0f;

    Vector3 cameraPos{.x = 0.0f, .y = 0.5f, .z = 0.0f};

    std::vector<SphereInstance> spheres;
};

} // export

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
    getApp()->setScene(nullptr);

    raytracePipeline = getApp()->getDevice()->createPipeline({
        .shader        = "res/shaders/raytrace",
        .vertexBuffers = {Vertex::layout()},
        .bindings =
            {
                {.binding = 0, .type = BindingType::UniformBuffer, .name = "RayTraceParams"},
                {.binding = 1, .type = BindingType::UniformBuffer, .name = "SphereBlock"},
            },
        .depthTest  = false,
        .depthWrite = false,
    });

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

    quadVbo = getApp()->getDevice()->createBuffer(BufferUsage::Vertex, std::span{quadVertices});

    initSpheres();
}

void RayTracingExample::onDetach() {
    raytracePipeline.reset();
    quadVbo.reset();
}

void RayTracingExample::onUpdate(float deltaTime) {
    time += deltaTime;

    if (spheres.empty()) {
        return;
    }

    // Hero sphere dynamic movement
    spheres[0].currentPos.x = spheres[0].basePos.x + (std::sin(time * spheres[0].animSpeed) * 1.6f);
    spheres[0].currentPos.y = spheres[0].basePos.y
                              + (std::cos(time * spheres[0].animSpeed * 1.3f) * spheres[0].animHeight);
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
    if (!raytracePipeline || !quadVbo) {
        return;
    }

    auto safeHeight = static_cast<float>(getApp()->getHeight());
    safeHeight      = std::max(safeHeight, 1.0f);

    int count = std::clamp(sphereCount, 1, static_cast<int>(spheres.size()));

    const Params params{
        .cameraPos          = Vector4{.x = cameraPos.x, .y = cameraPos.y, .z = cameraPos.z, .w = 0.0f},
        .aspectRatio        = static_cast<float>(getApp()->getWidth()) / safeHeight,
        .reflectionStrength = reflectionStrength,
        .sphereCount        = count,
        .enableShadows      = enableShadows ? 1 : 0,
        .enableReflections  = enableReflections ? 1 : 0,
    };

    SphereBlock sphereData{};
    for (std::size_t i = 0; std::cmp_less(i, count); ++i) {
        const Vector3& pos = spheres[i].currentPos;
        const Vector3& col = spheres[i].color;
        sphereData[i]      = {
            .posRad   = Vector4{.x = pos.x, .y = pos.y, .z = pos.z, .w = spheres[i].radius},
            .colShiny = Vector4{.x = col.x, .y = col.y, .z = col.z, .w = spheres[i].shininess},
        };
    }

    IRenderDevice* device = getApp()->getDevice();
    device->setPipeline(*raytracePipeline);
    device->setUniforms(0, params);
    device->setUniforms(1, sphereData);
    device->setVertexBuffer(0, *quadVbo);
    device->draw(6);

    ImGui::SetNextWindowPos(ImVec2(10.0f, 28.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(360, 480), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
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
