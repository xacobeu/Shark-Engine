export module shark:renderer.scene.node_types.Light;

import std;

import :renderer.scene.Node;

export namespace shark {
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
    auto* setDirection(this auto& self, Vector3 newDirection) {
        self.direction = normalize(newDirection);
        return &self;
    }
    auto* setRange(this auto& self, float newRange) {
        self.range = newRange;
        return &self;
    }

    // Light getters.
    [[nodiscard]] inline const Vector4& getColor() const { return color; }
    [[nodiscard]] inline LightType      getType() const { return type; }
    [[nodiscard]] inline float          getIntensity() const { return intensity; }
    [[nodiscard]] inline const Vector3& getDirection() const { return direction; }
    [[nodiscard]] inline float          getRange() const { return range; }

private:
    LightType type = LightType::Point;
    Vector4   color{.x = 1.0f, .y = 1.0f, .z = 1.0f, .w = 1.0f};
    Vector3   direction = normalize(Vector3{.x = -1.0f, .y = -1.0f, .z = -1.0f});
    float     intensity = 1.0f;
    float     range     = 10.0f;
};

} // namespace shark
