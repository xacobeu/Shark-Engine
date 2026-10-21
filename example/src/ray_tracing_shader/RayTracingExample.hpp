#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Layer.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/backends/IShader.hpp"
#include "renderer/backends/IVertexBuffer.hpp"
#include "util/Vector.hpp"

class RayTracingExample : public Layer {
public:
    static constexpr int MAX_SPHERES = 32;

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
    std::shared_ptr<IShader>       raytraceShader;
    std::shared_ptr<IVertexBuffer> quadVbo;

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
