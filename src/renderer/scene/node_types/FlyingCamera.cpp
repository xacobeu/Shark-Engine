module shark:renderer.scene.node_types.FlyingCamera.impl;

import std;

import :Application;
import :Input;
import :renderer.scene.node_types.FlyingCamera;
import :util.Vector;

namespace shark {

void FlyingCamera::update(Application* app, float deltaTime) {
    const bool imguiWantsMouse = Application::isMouseCaptured();

    double mouseX = app->getCursorX();
    double mouseY = app->getCursorY();

    // Free look rotation
    if (app->isMouseButtonPressed(MouseButton::Left) || app->isMouseButtonPressed(MouseButton::Right)) {
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

    if (app->isKeyPressed(Key::W)) {
        pos += direction * speed;
    }
    if (app->isKeyPressed(Key::S)) {
        pos -= direction * speed;
    }
    if (app->isKeyPressed(Key::A)) {
        pos -= right * speed;
    }
    if (app->isKeyPressed(Key::D)) {
        pos += right * speed;
    }
    if (app->isKeyPressed(Key::E) || app->isKeyPressed(Key::Space)) {
        pos += up * speed;
    }
    if (app->isKeyPressed(Key::Q) || app->isKeyPressed(Key::LeftShift)) {
        pos -= up * speed;
    }

    setPosition(pos);
}

} // namespace shark
