#include "Example.hpp"

#include "Application.hpp"
#include "imgui.h"
#include "renderer/geometry/MeshFactory.hpp"

#include <iostream>

void Example::onAttach() {
    std::cout << "[DEBUG] Example layer attached" << std::endl;
    app->getCamera().distance = 6.0f;
}

void Example::onUpdate(float deltaTime) {
    elapsed += deltaTime;
    ++frameCount;

    if (created) {
        cube.transform.rotationDeg.y += deltaTime * 35.0f;
    }
}

void Example::onRender(Renderer& renderer) {
    if (!created) {
        cube = renderer.createGameObject(MeshFactory::cube());
        cube.transform.scale = {1.0f, 1.0f, 1.0f};
        cube.transform.position = {0.0f, 0.0f, 0.0f};
        created = true;
    }

    renderer.beginScene(app->getCamera(), app->getWidth(), app->getHeight());
    renderer.submit(cube);

    ImGui::Begin("DEBUG");
    ImGui::Text("Elapsed: %.2f s", elapsed);
    ImGui::Text("Frames: %d", frameCount);
    ImGui::Text("Delta: %.4f s", app->getDeltaTime());

    if (ImGui::Button("Close App")) {
        app->close();
    }

    ImGui::End();
}
