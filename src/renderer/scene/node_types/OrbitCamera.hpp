#pragma once

#include <cmath>
#include <numbers>

#include "Camera.hpp"

class OrbitCamera : public Camera {
public:
    // Overrides.
    OrbitCamera* setPosition(Vector3 pos) {
        Node::setPosition(pos);
        return this;
    }
    OrbitCamera* setRotation(Vector3 rot) {
        Node::setRotation(rot);
        return this;
    }
    OrbitCamera* setScale(Vector3 scl) {
        Node::setScale(scl);
        return this;
    }
    OrbitCamera* setTransform(const Transform& trns) {
        Node::setTransform(trns);
        return this;
    }

    OrbitCamera* setFov(float newFov) {
        Camera::setFov(newFov);
        return this;
    }
    OrbitCamera* setNear(float newNear) {
        Camera::setNear(newNear);
        return this;
    }
    OrbitCamera* setFar(float newFar) {
        Camera::setFar(newFar);
        return this;
    }

    // Orbit setters.
    OrbitCamera* setTarget(Vector3 newTarget) {
        this->target = newTarget;
        return this;
    }
    OrbitCamera* setDistance(float newDistance) {
        this->distance = newDistance;
        return this;
    }

    // Orbit getters.
    [[nodiscard]] Vector3 getTarget() const { return target; }
    [[nodiscard]] float   getDistance() const { return distance; }

    // Camera matrices.
    [[nodiscard]] Matrix4 getViewMatrix() const override {
        float pitchRad = transform.rotationDeg.x * std::numbers::pi_v<float> / 180.0f;
        float yawRad   = transform.rotationDeg.y * std::numbers::pi_v<float> / 180.0f;

        Vector3 pos = {
            .x = target.x + (distance * std::cos(pitchRad) * std::cos(yawRad)),
            .y = target.y + (distance * std::sin(pitchRad)),
            .z = target.z + (distance * std::cos(pitchRad) * std::sin(yawRad))
        };
        return Matrix4::lookAt(pos, target, {.x = 0.0f, .y = 1.0f, .z = 0.0f});
    }

    void update(Application* app, float deltaTime) override;

private:
    Vector3 target{.x = 0.0f, .y = 0.0f, .z = 0.0f};
    float   distance = 10.0f;

    bool   isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    bool   isPanning = false;
    double lastPanX  = 0.0;
    double lastPanY  = 0.0;
};
