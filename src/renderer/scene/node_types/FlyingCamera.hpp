#pragma once

#include <cmath>
#include <numbers>

#include "Camera.hpp"

class FlyingCamera : public Camera {
public:
    // Overrides.
    FlyingCamera* setPosition(Vector3 pos) {
        Node::setPosition(pos);
        return this;
    }
    FlyingCamera* setRotation(Vector3 rot) {
        Node::setRotation(rot);
        return this;
    }
    FlyingCamera* setScale(Vector3 scl) {
        Node::setScale(scl);
        return this;
    }
    FlyingCamera* setTransform(const Transform& trns) {
        Node::setTransform(trns);
        return this;
    }

    FlyingCamera* setFov(float newFov) {
        Camera::setFov(newFov);
        return this;
    }
    FlyingCamera* setNear(float newNear) {
        Camera::setNear(newNear);
        return this;
    }
    FlyingCamera* setFar(float newFar) {
        Camera::setFar(newFar);
        return this;
    }

    [[nodiscard]] Matrix4 getViewMatrix() const override {
        float pitchRad = transform.rotationDeg.x * std::numbers::pi_v<float> / 180.0f;
        float yawRad   = transform.rotationDeg.y * std::numbers::pi_v<float> / 180.0f;

        Vector3 direction = {
            .x = std::cos(pitchRad) * std::cos(yawRad),
            .y = std::sin(pitchRad),
            .z = std::cos(pitchRad) * std::sin(yawRad)
        };

        float len = direction.length();
        if (len > 0.0f) {
            direction.x /= len;
            direction.y /= len;
            direction.z /= len;
        }

        Vector3 target = transform.position + direction;
        return Matrix4::lookAt(transform.position, target, {.x = 0, .y = 1, .z = 0});
    }

    void update(Application* app, float deltaTime) override;

private:
    bool   isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
};
