#pragma once

#include "../Node.hpp"
#include <memory>
#include <SharkEngine/renderer/backends/ITexture.hpp>
#include <SharkEngine/renderer/geometry/Mesh.hpp>
#include <SharkEngine/util/Vector.hpp>

class Entity : public Node {
public:
    // Overrides.
    Entity* setPosition(Vector3 pos) { Node::setPosition(pos); return this; }
    Entity* setRotation(Vector3 rot) { Node::setRotation(rot); return this; }
    Entity* setScale(Vector3 scl) { Node::setScale(scl); return this; }
    Entity* setTransform(const Transform& trns) { Node::setTransform(trns); return this; }

    Entity* setMesh(std::shared_ptr<Mesh> newMesh) { this->mesh = std::move(newMesh); return this; }
    Entity* setTexture(std::shared_ptr<ITexture> newTex) { this->texture = std::move(newTex); return this; }
    Entity* setColor(Vector4 newColor) { this->color = newColor; return this; }

    // Material getters.
    [[nodiscard]] const std::shared_ptr<Mesh>& getMesh() const { return mesh; }
    [[nodiscard]] const std::shared_ptr<ITexture>& getTexture() const { return texture; }
    [[nodiscard]] const Vector4& getColor() const { return color; }

private:
    std::shared_ptr<Mesh> mesh = nullptr;
    std::shared_ptr<ITexture> texture = nullptr;
    Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
};
