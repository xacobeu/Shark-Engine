#pragma once

#include <cstdint>

#include "../Node.hpp"

enum class LightType : std::uint8_t { Ambient, Directional, Point };

class Light : public Node {
public:
    // Light setters.
    auto* setColor(this auto& self, Vector4 newColor) {
        self.color = newColor;
        return &self;
    }
    auto* setType(this auto& self, LightType newType) {
        self.type = newType;
        return &self;
    }
    auto* setIntensity(this auto& self, float newIntensity) {
        self.intensity = newIntensity;
        return &self;
    }

    // Light getters.
    [[nodiscard]] const Vector4& getColor() const { return color; }
    [[nodiscard]] LightType      getType() const { return type; }
    [[nodiscard]] float          getIntensity() const { return intensity; }

private:
    LightType type = LightType::Point;
    Vector4   color{.x = 1.0f, .y = 1.0f, .z = 1.0f, .w = 1.0f};
    float     intensity = 1.0f;
};
