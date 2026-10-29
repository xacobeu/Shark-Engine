export module shark:renderer.scene.node_types.Camera;

import :renderer.scene.Node;
import :util.Matrix4;

export namespace shark {
class Application;

class Camera : public Node {
public:
    // Camera setters.
    auto* setFov(this auto& self, float newFov) {
        self.fov = newFov;
        return &self;
    }
    auto* setNear(this auto& self, float newNear) {
        self.near = newNear;
        return &self;
    }
    auto* setFar(this auto& self, float newFar) {
        self.far = newFar;
        return &self;
    }

    // Camera getters.
    [[nodiscard]] inline float getFov() const { return fov; }
    [[nodiscard]] inline float getNear() const { return near; }
    [[nodiscard]] inline float getFar() const { return far; }

    // Camera matrices.
    [[nodiscard]] inline virtual Matrix4 getProjectionMatrix(float aspect) const {
        return Matrix4::perspective({.fovDeg = fov, .aspect = aspect, .nearZ = near, .farZ = far});
    }

    [[nodiscard]] inline virtual Vector3 getEyePosition() const { return getPosition(); }

    [[nodiscard]] inline virtual Matrix4 getViewMatrix() const {
        return Matrix4::lookAt(getPosition(), {.x = 0, .y = 0, .z = 0}, {.x = 0, .y = 1, .z = 0});
    }

    inline virtual void update(Application* /*app*/, float /*deltaTime*/) {}

private:
    float fov  = 45.0f;
    float near = 0.1f;
    float far  = 1000.0f;
};

} // namespace shark
