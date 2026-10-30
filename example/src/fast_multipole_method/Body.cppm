export module examples.fast_multipole_method.body;

import shark;

using namespace shark;

export {
struct alignas(16) Body {
    Vector3 position{0.0f, 0.0f, 0.0f};
    Vector3 velocity{0.0f, 0.0f, 0.0f};
    Vector3 acceleration{0.0f, 0.0f, 0.0f};
    float   mass = 1.0f;

    inline void integrate(float dt) noexcept {
        velocity += acceleration * dt;
        position += velocity * dt;
        acceleration = {0.0f, 0.0f, 0.0f};
    }
};

} // export
