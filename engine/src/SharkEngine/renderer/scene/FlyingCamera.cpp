#include <GLFW/glfw3.h>
#include <SharkEngine/Application.hpp>
#include <SharkEngine/renderer/scene/node_types/FlyingCamera.hpp>
#include <cmath>
#include <numbers>

void FlyingCamera::update(Application *app, float deltaTime) {
    double mouseX = app->getCursorX();
    double mouseY = app->getCursorY();

    // Free look rotation
    if (app->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) || app->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (!isDragging) {
            isDragging = true;
            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }

        double  dx = mouseX - lastMouseX;
        double  dy = mouseY - lastMouseY;
        Vector3 rot = getRotation();
        rot.y += static_cast<float>(dx) * 0.15f;
        rot.x -= static_cast<float>(dy) * 0.15f;

        if (rot.x > 89.0f)
            rot.x = 89.0f;
        if (rot.x < -89.0f)
            rot.x = -89.0f;

        setRotation(rot);

        lastMouseX = mouseX;
        lastMouseY = mouseY;
    } else {
        isDragging = false;
    }

    // Free flight controls
    float speed = 5.0f * deltaTime;

    float pitchRad = transform.rotationDeg.x * std::numbers::pi_v<float> / 180.0f;
    float yawRad = transform.rotationDeg.y * std::numbers::pi_v<float> / 180.0f;

    Vector3 direction = { std::cos(pitchRad) * std::cos(yawRad), std::sin(pitchRad), std::cos(pitchRad) * std::sin(yawRad) };

    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (len > 0.0f) {
        direction.x /= len;
        direction.y /= len;
        direction.z /= len;
    }

    Vector3 right = { -std::sin(yawRad), 0.0f, std::cos(yawRad) };

    Vector3 up = { 0.0f, 1.0f, 0.0f };

    Vector3 pos = getPosition();

    if (app->isKeyPressed(GLFW_KEY_W)) {
        pos = pos + direction * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_S)) {
        pos = pos - direction * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_A)) {
        pos = pos - right * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_D)) {
        pos = pos + right * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_E) || app->isKeyPressed(GLFW_KEY_SPACE)) {
        pos = pos + up * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_Q) || app->isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        pos = pos - up * speed;
    }

    setPosition(pos);
}
