#include <SharkEngine/Application.hpp>
#include <SharkEngine/renderer/scene/node_types/OrbitCamera.hpp>
#include <GLFW/glfw3.h>
#include <cmath>

void OrbitCamera::update(Application* app, float) {
    // Zoom
    double scrollY = app->getScrollY();
    if (scrollY != 0.0) {
        distance -= static_cast<float>(scrollY) * 0.5f;
        if (distance < 1.0f) distance = 1.0f;
        if (distance > 100.0f) distance = 100.0f;
    }

    double mouseX = app->getCursorX();
    double mouseY = app->getCursorY();

    // Rotation
    if (app->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        if (!isDragging) {
            isDragging = true;
            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }

        double dx = mouseX - lastMouseX;
        double dy = mouseY - lastMouseY;

        Vector3 rot = getRotation();
        rot.y += static_cast<float>(dx) * 0.25f;
        rot.x += static_cast<float>(dy) * 0.25f;

        if (rot.x > 89.0f) rot.x = 89.0f;
        if (rot.x < -89.0f) rot.x = -89.0f;

        setRotation(rot);

        lastMouseX = mouseX;
        lastMouseY = mouseY;
    } else {
        isDragging = false;
    }

    // Pan
    if (app->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (!isPanning) {
            isPanning = true;
            lastPanX = mouseX;
            lastPanY = mouseY;
        }

        double dx = mouseX - lastPanX;
        double dy = mouseY - lastPanY;

        float yawRad = transform.rotationDeg.y * 3.14159265f / 180.0f;

        Vector3 right = { -std::sin(yawRad), 0.0f, std::cos(yawRad) };
        Vector3 up = { 0.0f, 1.0f, 0.0f };

        float panSpeed = distance * 0.002f;
        
        target = target - right * (static_cast<float>(dx) * panSpeed) + up * (static_cast<float>(dy) * panSpeed);

        lastPanX = mouseX;
        lastPanY = mouseY;
    } else {
        isPanning = false;
    }
}
