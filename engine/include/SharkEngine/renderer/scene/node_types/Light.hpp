#pragma once

#include "../Node.hpp"

enum class LightType { Ambient, Directional, Point };

class Light : public Node {
public:
    // Overrides.
    Light* setPosition(Vector3 pos) { Node::setPosition(pos); return this; }
    Light* setRotation(Vector3 rot) { Node::setRotation(rot); return this; }
    Light* setScale(Vector3 scl) { Node::setScale(scl); return this; }
    Light* setTransform(const Transform& trns) { Node::setTransform(trns); return this; }

    // Light setters.
    Light* setColor(Vector4 newColor) { this->color = newColor; return this; }
    Light* setType(LightType newType) { this->type = newType; return this; }
    Light* setIntensity(float newIntensity) { this->intensity = newIntensity; return this; }

    // Light getters.
    [[nodiscard]] Vector4 getColor() const { return color; }
    [[nodiscard]] LightType getType() const { return type; }
    [[nodiscard]] float getIntensity() const { return intensity; }

private:
    LightType type;
    Vector4 color{1.0f, 1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
};
