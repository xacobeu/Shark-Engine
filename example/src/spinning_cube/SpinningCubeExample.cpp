#include "SpinningCubeExample.hpp"

#include <memory>

#include "Application.hpp"
#include "renderer/geometry/MeshFactory.hpp"

void SpinningCubeExample::onAttach() {
    getApp()->setScene(&scene);

    camera = scene.add<OrbitCamera>()->setDistance(6.0f);

    cube = scene.add<Entity>()
               ->setMesh(std::make_shared<Mesh>(MeshFactory::cube()))
               ->setScale({.x = 1.0f, .y = 1.0f, .z = 1.0f})
               ->setPosition({.x = 0.0f, .y = 0.0f, .z = 0.0f})
               ->setTexture(getApp()->getDevice()->createTexture("res/textures/scifi_crate.png"));
}

void SpinningCubeExample::onDetach() {
    if ((getApp() != nullptr) && getApp()->getScene() == &scene) {
        getApp()->setScene(nullptr);
    }
    cube   = nullptr;
    camera = nullptr;
    scene.clear();
}

void SpinningCubeExample::onUpdate(float deltaTime) {
    elapsed += deltaTime;
    ++frameCount;

    cube->setRotation({.x = 0.0f, .y = cube->getRotation().y + (deltaTime * 35.0f), .z = 0.0f});
}

void SpinningCubeExample::onRender(Renderer& renderer) {
    renderer.render(scene, getApp()->getWidth(), getApp()->getHeight());
}
