#pragma once

#include "Camera.hpp"
#include <cmath>

class FlyingCamera : public Camera {
public:
    // Overrides.
    FlyingCamera* setPosition(Vector3 pos) { Node::setPosition(pos); return this; }
    FlyingCamera* setRotation(Vector3 rot) { Node::setRotation(rot); return this; }
    FlyingCamera* setScale(Vector3 scl) { Node::setScale(scl); return this; }
    FlyingCamera* setTransform(const Transform& trns) { Node::setTransform(trns); return this; }

    FlyingCamera* setFov(float newFov) { Camera::setFov(newFov); return this; }
    FlyingCamera* setNear(float newNear) { Camera::setNear(newNear); return this; }
    FlyingCamera* setFar(float newFar) { Camera::setFar(newFar); return this; }

    // Camera matrices.
    [[nodiscard]] Matrix4 getViewMatrix() const override {
        float pitchRad = transform.rotationDeg.x * 3.14159265f / 180.0f;
        float yawRad = transform.rotationDeg.y * 3.14159265f / 180.0f;

        Vector3 direction = {
            cos(pitchRad) * cos(yawRad),
            sin(pitchRad),
            cos(pitchRad) * sin(yawRad)
        };

        float len = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
        if (len > 0.0f) {
            direction.x /= len;
            direction.y /= len;
            direction.z /= len;
        }

        Vector3 target = transform.position + direction;
        return Matrix4::lookAt(transform.position, target, {0, 1, 0});
    }

    void update(Application* app, float deltaTime) override;

private:
    bool isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
};
