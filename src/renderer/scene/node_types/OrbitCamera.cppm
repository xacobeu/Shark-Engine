export module shark:renderer.scene.node_types.OrbitCamera;

import std;

import :Application;
import :Input;
import :renderer.scene.node_types.Camera;
import :util.Vector;

export namespace shark {
class OrbitCamera : public Camera {
public:
    // Orbit setters.
    inline auto* setTarget(this auto& self, Vector3 newTarget) {
        self.target = newTarget;
        return &self;
    }
    inline auto* setDistance(this auto& self, float newDistance) {
        self.distance = newDistance;
        return &self;
    }

    inline auto* setRotateButton(this auto& self, MouseButton button) {
        self.rotateButton = button;
        return &self;
    }
    inline auto* setPanButton(this auto& self, MouseButton button) {
        self.panButton = button;
        return &self;
    }
    inline auto* setRotationEnabled(this auto& self, bool enabled) {
        self.rotationEnabled = enabled;
        return &self;
    }
    inline auto* setPanEnabled(this auto& self, bool enabled) {
        self.panEnabled = enabled;
        return &self;
    }

    // Orbit getters.
    [[nodiscard]] inline const Vector3& getTarget() const { return target; }
    [[nodiscard]] inline float          getDistance() const { return distance; }
    [[nodiscard]] inline MouseButton    getRotateButton() const { return rotateButton; }
    [[nodiscard]] inline MouseButton    getPanButton() const { return panButton; }
    [[nodiscard]] inline bool           isRotationEnabled() const { return rotationEnabled; }
    [[nodiscard]] inline bool           isPanEnabled() const { return panEnabled; }

    // Camera matrices.
    [[nodiscard]] inline Vector3 getEyePosition() const override {
        float pitchRad = getRotation().x * std::numbers::pi_v<float> / 180.0f;
        float yawRad   = getRotation().y * std::numbers::pi_v<float> / 180.0f;

        return {
            .x = target.x + (distance * std::cos(pitchRad) * std::cos(yawRad)),
            .y = target.y + (distance * std::sin(pitchRad)),
            .z = target.z + (distance * std::cos(pitchRad) * std::sin(yawRad))
        };
    }

    [[nodiscard]] inline Matrix4 getViewMatrix() const override {
        return Matrix4::lookAt(getEyePosition(), target, {.x = 0.0f, .y = 1.0f, .z = 0.0f});
    }

    void update(Application* app, float deltaTime) override;

private:
    Vector3 target{.x = 0.0f, .y = 0.0f, .z = 0.0f};
    float   distance = 10.0f;

    MouseButton rotateButton    = MouseButton::Left;
    MouseButton panButton       = MouseButton::Right;
    bool        rotationEnabled = true;
    bool        panEnabled      = true;

    bool   isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    bool   isPanning = false;
    double lastPanX  = 0.0;
    double lastPanY  = 0.0;
};

} // namespace shark

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
