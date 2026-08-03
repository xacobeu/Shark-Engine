#include "BoxExample.hpp"
#include "imgui.h"
#include <SharkEngine/Application.hpp>
#include <SharkEngine/renderer/geometry/MeshFactory.hpp>
#include <iostream>

void BoxExample::onAttach() {
    std::cout << "[DEBUG] BoxExample layer attached" << std::endl;

    app->setScene(&scene);

    camera = scene.add<FlyingCamera>()
        ->setPosition({0.0f, 0.0f, 5.5f})
        ->setRotation({0.0f, -90.0f, 0.0f});

    auto cubeMesh = std::make_shared<Mesh>(MeshFactory::cube());

    // Cornell Box Walls
    // Floor (White)
    scene.add<Entity>()
        ->setMesh(cubeMesh)
        ->setPosition({0.0f, -2.0f, 0.0f})
        ->setScale({4.0f, 0.1f, 4.0f})
        ->setColor({0.8f, 0.8f, 0.8f, 1.0f});

    // Ceiling (White)
    scene.add<Entity>()
        ->setMesh(cubeMesh)
        ->setPosition({0.0f, 2.0f, 0.0f})
        ->setScale({4.0f, 0.1f, 4.0f})
        ->setColor({0.8f, 0.8f, 0.8f, 1.0f});

    // Back Wall (White)
    scene.add<Entity>()
        ->setMesh(cubeMesh)
        ->setPosition({0.0f, 0.0f, -2.0f})
        ->setScale({4.0f, 4.0f, 0.1f})
        ->setColor({0.8f, 0.8f, 0.8f, 1.0f});

    // Left Wall (Red)
    scene.add<Entity>()
        ->setMesh(cubeMesh)
        ->setPosition({-2.0f, 0.0f, 0.0f})
        ->setScale({0.1f, 4.0f, 4.0f})
        ->setColor({0.8f, 0.1f, 0.1f, 1.0f});

    // Right Wall (Green)
    scene.add<Entity>()
        ->setMesh(cubeMesh)
        ->setPosition({2.0f, 0.0f, 0.0f})
        ->setScale({0.1f, 4.0f, 4.0f})
        ->setColor({0.1f, 0.8f, 0.1f, 1.0f});

    // Emissive Light Source on Ceiling (Yellow/White)
    scene.add<Entity>()
        ->setMesh(cubeMesh)
        ->setPosition({0.0f, 1.95f, 0.0f})
        ->setScale({1.0f, 0.02f, 1.0f})
        ->setColor({1.0f, 1.0f, 0.9f, 1.0f});

    // Inner Blocks
    // Short Box (White/Light Gray)
    scene.add<Entity>()
        ->setMesh(cubeMesh)
        ->setPosition({0.5f, -1.4f, 0.5f})
        ->setScale({1.2f, 1.2f, 1.2f})
        ->setRotation({0.0f, -18.0f, 0.0f})
        ->setColor({0.75f, 0.75f, 0.75f, 1.0f});

    // Tall Box (White/Light Gray)
    tallBox = scene.add<Entity>()
        ->setMesh(cubeMesh)
        ->setPosition({-0.5f, -0.8f, -0.3f})
        ->setScale({1.2f, 2.4f, 1.2f})
        ->setRotation({0.0f, 15.0f, 0.0f})
        ->setColor({0.75f, 0.75f, 0.75f, 1.0f});
}

void BoxExample::onUpdate(float deltaTime) {
    float rawDeltaTime = deltaTime;
    if (rawDeltaTime > 0.1f) {
        rawDeltaTime = 0.1f;
    }

    fpsTimer += rawDeltaTime;
    if (fpsTimer >= 0.5f) {
        displayedFps = ImGui::GetIO().Framerate;
        fpsTimer = 0.0f;
    }
}

void BoxExample::onRender(Renderer& renderer) {
    renderer.render(scene, app->getWidth(), app->getHeight());

    ImGui::Begin("Cornell Box Controller");
    ImGui::Text("FPS: %.1f", displayedFps);
    ImGui::Text("Camera Pos: X:%.2f, Y:%.2f, Z:%.2f", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
    ImGui::Text("Camera Rot: Yaw:%.1f, Pitch:%.1f", camera->getRotation().y, camera->getRotation().x);
    ImGui::End();
}
