#pragma once

#include <memory>

#include "Layer.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/backends/IShader.hpp"
#include "renderer/backends/IVertexBuffer.hpp"
#include "util/Vector.hpp"

class RayTracingExample : public Layer {
public:
    void onAttach() override;
    void onDetach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

private:
    std::shared_ptr<IShader>       raytraceShader;
    std::shared_ptr<IVertexBuffer> quadVbo;

    float time               = 0.0f;
    bool  enableShadows      = true;
    bool  enableReflections  = true;
    float reflectionStrength = 0.2f;

    Vector3 sphere1Pos{.x = 0.0f, .y = 0.0f, .z = -5.0f};
    float   sphere1Radius = 1.0f;
    Vector3 cameraPos{.x = 0.0f, .y = 0.0f, .z = 0.0f};
};
