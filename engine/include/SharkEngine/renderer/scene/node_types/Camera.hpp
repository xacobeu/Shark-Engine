#pragma once

#include "../Node.hpp"
#include <SharkEngine/util/Matrix4.hpp>
#include <SharkEngine/util/Vector.hpp>

class Camera : public Node {
public:
    // Overrides.
    Camera* setPosition(Vector3 pos) { Node::setPosition(pos); return this; }
    Camera* setRotation(Vector3 rot) { Node::setRotation(rot); return this; }
    Camera* setScale(Vector3 scl) { Node::setScale(scl); return this; }
    Camera* setTransform(const Transform& trns) { Node::setTransform(trns); return this; }

    // Camera setters.
    Camera* setFov(float newFov) { this->fov = newFov; return this; }
    Camera* setNear(float newNear) { this->near = newNear; return this; }
    Camera* setFar(float newFar) { this->far = newFar; return this; }

    // Camera getters.
    [[nodiscard]] float getFov() const { return fov; }
    [[nodiscard]] float getNear() const { return near; }
    [[nodiscard]] float getFar() const { return far; }

    // Camera matrices.
    [[nodiscard]] virtual Matrix4 getProjectionMatrix(float aspect) const {
        return Matrix4::perspective(fov, aspect, near, far);
    }
    
    [[nodiscard]] virtual Matrix4 getViewMatrix() const {
        return Matrix4::lookAt(transform.position, {0, 0, 0}, {0, 1, 0});
    }

private:
    float fov = 45.0f;
    float near = 0.1f;
    float far = 1000.0f;
};
