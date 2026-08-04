#pragma once

import SharkEngine;

class SpinningCubeExample : public Layer {
public:
    void onAttach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

private:
    Scene scene;
    Entity* cube = nullptr;
    OrbitCamera* camera = nullptr;
    float elapsed = 0.0f;
    int frameCount = 0;
};
