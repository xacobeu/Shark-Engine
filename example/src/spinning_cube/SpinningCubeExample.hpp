#pragma once

#include "Layer.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/scene/Scene.hpp"
#include "renderer/scene/node_types/Entity.hpp"
#include "renderer/scene/node_types/OrbitCamera.hpp"

class SpinningCubeExample : public Layer {
public:
    void onAttach() override;
    void onDetach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

private:
    Scene        scene;
    Entity*      cube       = nullptr;
    OrbitCamera* camera     = nullptr;
    float        elapsed    = 0.0f;
    int          frameCount = 0;
};
