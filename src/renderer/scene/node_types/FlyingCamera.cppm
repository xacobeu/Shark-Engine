export module shark:renderer.scene.node_types.FlyingCamera;

import std;

import :renderer.scene.node_types.Camera;

export namespace shark {
class FlyingCamera : public Camera {
public:
    [[nodiscard]] inline Matrix4 getViewMatrix() const override {
        float pitchRad = getRotation().x * std::numbers::pi_v<float> / 180.0f;
        float yawRad   = getRotation().y * std::numbers::pi_v<float> / 180.0f;

        Vector3 direction = normalize(
            Vector3{
                .x = std::cos(pitchRad) * std::cos(yawRad),
                .y = std::sin(pitchRad),
                .z = std::cos(pitchRad) * std::sin(yawRad)
            }
        );

        Vector3 target = getPosition() + direction;
        return Matrix4::lookAt(getPosition(), target, {.x = 0, .y = 1, .z = 0});
    }

    void update(Application* app, float deltaTime) override;

private:
    bool   isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
};

} // namespace shark
