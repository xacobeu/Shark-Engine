#include "SpinningCubeExample.hpp"

#include "imgui.h"
#include <SharkEngine/Application.hpp>
#include <SharkEngine/renderer/geometry/MeshFactory.hpp>

#include <iostream>

void SpinningCubeExample::onAttach() {
	std::cout << "[DEBUG] SpinningCubeExample layer attached" << std::endl;

	app->setScene(&scene);

	camera = scene.add<OrbitCamera>()
		->setDistance(6.0f);

	cube = scene.add<Entity>()
		->setMesh(std::make_shared<Mesh>(MeshFactory::cube()))
		->setScale({1.0f, 1.0f, 1.0f})
		->setPosition({0.0f, 0.0f, 0.0f})
		->setTexture(std::make_shared<Texture>("res/textures/scifi_crate.png"));
}

void SpinningCubeExample::onUpdate(float deltaTime) {
	elapsed += deltaTime;
	++frameCount;

	cube->setRotation({0.0f, cube->getRotation().y + deltaTime * 35.0f, 0.0f});
}

void SpinningCubeExample::onRender(Renderer &renderer) {

	renderer.render(scene, app->getWidth(), app->getHeight());

	ImGui::Begin("DEBUG");
	ImGui::Text("Elapsed: %.2f s", elapsed);
	ImGui::Text("Frames: %d", frameCount);
	ImGui::Text("Delta: %.4f s", app->getDeltaTime());

	if (ImGui::Button("Close App")) {
		app->close();
	}

	ImGui::End();
}
