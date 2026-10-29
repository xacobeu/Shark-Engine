#include "renderer/scene/node_types/FlyingCamera.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

#include "imgui.h"
#include <GLFW/glfw3.h>

#include "Application.hpp"

void FlyingCamera::update(Application* app, float deltaTime) {
    const bool imguiWantsMouse = (ImGui::GetCurrentContext() != nullptr) && ImGui::GetIO().WantCaptureMouse;

    double mouseX = app->getCursorX();
    double mouseY = app->getCursorY();

    // Free look rotation
    if (app->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) || app->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (!isDragging) {
            if (!imguiWantsMouse) {
                isDragging = true;
                lastMouseX = mouseX;
                lastMouseY = mouseY;
            }
        } else {
            double  dx  = mouseX - lastMouseX;
            double  dy  = mouseY - lastMouseY;
            Vector3 rot = getRotation();
            rot.y += static_cast<float>(dx) * 0.15f;
            rot.x -= static_cast<float>(dy) * 0.15f;

            rot.x = std::min(rot.x, 89.0f);
            rot.x = std::max(rot.x, -89.0f);

            setRotation(rot);

            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }
    } else {
        isDragging = false;
    }

    // Free flight controls
    float speed = 5.0f * deltaTime;

    float pitchRad = getRotation().x * std::numbers::pi_v<float> / 180.0f;
    float yawRad   = getRotation().y * std::numbers::pi_v<float> / 180.0f;

    Vector3 direction = normalize(
        Vector3{
            .x = std::cos(pitchRad) * std::cos(yawRad),
            .y = std::sin(pitchRad),
            .z = std::cos(pitchRad) * std::sin(yawRad),
        }
    );

    Vector3 right = {.x = -std::sin(yawRad), .y = 0.0f, .z = std::cos(yawRad)};
    Vector3 up    = {.x = 0.0f, .y = 1.0f, .z = 0.0f};
    Vector3 pos   = getPosition();

    if (app->isKeyPressed(GLFW_KEY_W)) {
        pos += direction * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_S)) {
        pos -= direction * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_A)) {
        pos -= right * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_D)) {
        pos += right * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_E) || app->isKeyPressed(GLFW_KEY_SPACE)) {
        pos += up * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_Q) || app->isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        pos -= up * speed;
    }

    setPosition(pos);
}
