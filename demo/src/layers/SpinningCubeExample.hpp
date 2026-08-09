#pragma once

#include <SharkEngine/Layer.hpp>
#include <SharkEngine/renderer/scene/Scene.hpp>
#include <SharkEngine/renderer/scene/node_types/Entity.hpp>
#include <SharkEngine/renderer/scene/node_types/OrbitCamera.hpp>
#include <SharkEngine/renderer/geometry/MeshFactory.hpp>
#include <SharkEngine/renderer/geometry/Mesh.hpp>
#include <SharkEngine/renderer/Renderer.hpp>
#include <SharkEngine/Application.hpp>

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
