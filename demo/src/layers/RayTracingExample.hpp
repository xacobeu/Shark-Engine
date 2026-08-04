#pragma once

#include <memory>
import SharkEngine;

class RayTracingExample : public Layer {
public:
    void onAttach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

private:
    std::shared_ptr<IShader> raytraceShader;
    std::shared_ptr<IVertexBuffer> quadVbo;
    
    float time = 0.0f;
    bool enableShadows = true;
    bool enableReflections = true;
    float reflectionStrength = 0.2f;
    
    Vector3 sphere1Pos{0.0f, 0.0f, -5.0f};
    float sphere1Radius = 1.0f;
    Vector3 cameraPos{0.0f, 0.0f, 0.0f};
};
