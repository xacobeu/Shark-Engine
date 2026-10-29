export module shark:renderer.scene.node_types.Entity;

import std;

import :renderer.scene.Node;
import :renderer.backends.ITexture;
import :renderer.geometry.Mesh;
import :util.Vector;

export namespace shark {
class Entity : public Node {
public:
    auto* setMesh(this auto& self, std::shared_ptr<Mesh> newMesh) {
        self.mesh = std::move(newMesh);
        return &self;
    }
    auto* setTexture(this auto& self, std::shared_ptr<ITexture> newTex) {
        self.texture = std::move(newTex);
        return &self;
    }
    auto* setColor(this auto& self, Vector4 newColor) {
        self.color = newColor;
        return &self;
    }
    auto* setSpecular(this auto& self, float newSpecular) {
        self.specular = newSpecular;
        return &self;
    }
    auto* setShininess(this auto& self, float newShininess) {
        self.shininess = newShininess;
        return &self;
    }

    // Material getters.
    [[nodiscard]] inline const std::shared_ptr<Mesh>&     getMesh() const { return mesh; }
    [[nodiscard]] inline const std::shared_ptr<ITexture>& getTexture() const { return texture; }
    [[nodiscard]] inline const Vector4&                   getColor() const { return color; }
    [[nodiscard]] inline float                            getSpecular() const { return specular; }
    [[nodiscard]] inline float                            getShininess() const { return shininess; }

    static constexpr float DEFAULT_SPECULAR  = 0.4f;
    static constexpr float DEFAULT_SHININESS = 32.0f;

private:
    std::shared_ptr<Mesh>     mesh      = nullptr;
    std::shared_ptr<ITexture> texture   = nullptr;
    Vector4                   color     = {1.0f, 1.0f, 1.0f, 1.0f};
    float                     specular  = DEFAULT_SPECULAR;
    float                     shininess = DEFAULT_SHININESS;
};

} // namespace shark
