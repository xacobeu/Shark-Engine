#pragma once

#include "util/Vector.hpp"

struct Transform {
    Vector3 position{0.0f, 0.0f, 0.0f};
    Vector3 rotationDeg{0.0f, 0.0f, 0.0f};
    Vector3 scale{1.0f, 1.0f, 1.0f};
};

class Node {
public:
    virtual ~Node() = default;

    Node(const Node&)            = delete;
    Node& operator=(const Node&) = delete;
    Node(Node&&)                 = delete;
    Node& operator=(Node&&)      = delete;

    // Transform setters.
    auto* setPosition(this auto& self, Vector3 pos) {
        self.transform.position = pos;
        return &self;
    }
    auto* setRotation(this auto& self, Vector3 rot) {
        self.transform.rotationDeg = rot;
        return &self;
    }
    auto* setScale(this auto& self, Vector3 scl) {
        self.transform.scale = scl;
        return &self;
    }
    auto* setTransform(this auto& self, const Transform& trns) {
        self.transform = trns;
        return &self;
    }

    // Transform getters.
    [[nodiscard]] const Vector3&   getPosition() const { return transform.position; }
    [[nodiscard]] const Vector3&   getRotation() const { return transform.rotationDeg; }
    [[nodiscard]] const Vector3&   getScale() const { return transform.scale; }
    [[nodiscard]] const Transform& getTransform() const { return transform; }

protected:
    Node() = default;

private:
    Transform transform;
};
