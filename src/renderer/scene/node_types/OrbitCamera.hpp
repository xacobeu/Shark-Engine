#pragma once

#include <cmath>
#include <numbers>

#include "Camera.hpp"

class OrbitCamera : public Camera {
public:
    // Orbit setters.
    auto* setTarget(this auto& self, Vector3 newTarget) {
        self.target = newTarget;
        return &self;
    }
    auto* setDistance(this auto& self, float newDistance) {
        self.distance = newDistance;
        return &self;
    }

    auto* setRotateButton(this auto& self, int button) {
        self.rotateButton = button;
        return &self;
    }
    auto* setPanButton(this auto& self, int button) {
        self.panButton = button;
        return &self;
    }
    auto* setRotationEnabled(this auto& self, bool enabled) {
        self.rotationEnabled = enabled;
        return &self;
    }
    auto* setPanEnabled(this auto& self, bool enabled) {
        self.panEnabled = enabled;
        return &self;
    }

    // Orbit getters.
    [[nodiscard]] const Vector3& getTarget() const { return target; }
    [[nodiscard]] float          getDistance() const { return distance; }
    [[nodiscard]] int            getRotateButton() const { return rotateButton; }
    [[nodiscard]] int            getPanButton() const { return panButton; }
    [[nodiscard]] bool           isRotationEnabled() const { return rotationEnabled; }
    [[nodiscard]] bool           isPanEnabled() const { return panEnabled; }

    // Camera matrices.
    [[nodiscard]] Matrix4 getViewMatrix() const override {
        float pitchRad = getRotation().x * std::numbers::pi_v<float> / 180.0f;
        float yawRad   = getRotation().y * std::numbers::pi_v<float> / 180.0f;

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

    int  rotateButton    = 0; // GLFW_MOUSE_BUTTON_LEFT (0)
    int  panButton       = 1; // GLFW_MOUSE_BUTTON_RIGHT (1)
    bool rotationEnabled = true;
    bool panEnabled      = true;

    bool   isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    bool   isPanning = false;
    double lastPanX  = 0.0;
    double lastPanY  = 0.0;
};
