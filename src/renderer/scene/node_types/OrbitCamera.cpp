module shark:renderer.scene.node_types.OrbitCamera.impl;

import std;

import :Application;
import :renderer.scene.node_types.OrbitCamera;
import :util.Vector;

namespace shark {

void OrbitCamera::update(Application* app, float) {
    const bool imguiWantsMouse = Application::isMouseCaptured();

    // Zoom
    double scrollY = app->getScrollY();
    if (!imguiWantsMouse && scrollY != 0.0) {
        distance -= static_cast<float>(scrollY) * 0.5f;
        distance = std::max(distance, 1.0f);
        distance = std::min(distance, 600.0f);
    }

    double mouseX = app->getCursorX();
    double mouseY = app->getCursorY();

    // Rotation
    if (rotationEnabled && app->isMouseButtonPressed(rotateButton)) {
        if (!isDragging) {
            if (!imguiWantsMouse) {
                isDragging = true;
                lastMouseX = mouseX;
                lastMouseY = mouseY;
            }
        } else {
            double dx = mouseX - lastMouseX;
            double dy = mouseY - lastMouseY;

            Vector3 rot = getRotation();
            rot.y += static_cast<float>(dx) * 0.25f;
            rot.x += static_cast<float>(dy) * 0.25f;

            rot.x = std::min(rot.x, 89.0f);
            rot.x = std::max(rot.x, -89.0f);

            setRotation(rot);

            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }
    } else {
        isDragging = false;
    }

    // Pan
    if (panEnabled && app->isMouseButtonPressed(panButton)) {
        if (!isPanning) {
            if (!imguiWantsMouse) {
                isPanning = true;
                lastPanX  = mouseX;
                lastPanY  = mouseY;
            }
        } else {
            double dx = mouseX - lastPanX;
            double dy = mouseY - lastPanY;

            float yawRad = getRotation().y * std::numbers::pi_v<float> / 180.0f;

            Vector3 right = {.x = -std::sin(yawRad), .y = 0.0f, .z = std::cos(yawRad)};
            Vector3 up    = {.x = 0.0f, .y = 1.0f, .z = 0.0f};

            float panSpeed = distance * 0.002f;

            target += (up * (static_cast<float>(dy) * panSpeed)) - (right * (static_cast<float>(dx) * panSpeed));

            lastPanX = mouseX;
            lastPanY = mouseY;
        }
    } else {
        isPanning = false;
    }
}

} // namespace shark
