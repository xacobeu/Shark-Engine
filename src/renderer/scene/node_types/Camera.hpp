#pragma once

#include "../Node.hpp"
#include "util/Matrix4.hpp"
#include "util/Vector.hpp"

class Application;

class Camera : public Node {
public:
    // Overrides.
    Camera* setPosition(Vector3 pos) {
        Node::setPosition(pos);
        return this;
    }
    Camera* setRotation(Vector3 rot) {
        Node::setRotation(rot);
        return this;
    }
    Camera* setScale(Vector3 scl) {
        Node::setScale(scl);
        return this;
    }
    Camera* setTransform(const Transform& trns) {
        Node::setTransform(trns);
        return this;
    }

    // Camera setters.
    Camera* setFov(float newFov) {
        this->fov = newFov;
        return this;
    }
    Camera* setNear(float newNear) {
        this->near = newNear;
        return this;
    }
    Camera* setFar(float newFar) {
        this->far = newFar;
        return this;
    }

    // Camera getters.
    [[nodiscard]] float getFov() const { return fov; }
    [[nodiscard]] float getNear() const { return near; }
    [[nodiscard]] float getFar() const { return far; }

    // Camera matrices.
    [[nodiscard]] virtual Matrix4 getProjectionMatrix(float aspect) const {
        return Matrix4::perspective({.fovDeg = fov, .aspect = aspect, .nearZ = near, .farZ = far});
    }

    [[nodiscard]] virtual Matrix4 getViewMatrix() const {
        return Matrix4::lookAt(transform.position, {.x = 0, .y = 0, .z = 0}, {.x = 0, .y = 1, .z = 0});
    }

    virtual void update(Application* /*app*/, float /*deltaTime*/) {}

private:
    float fov  = 45.0f;
    float near = 0.1f;
    float far  = 1000.0f;
};
