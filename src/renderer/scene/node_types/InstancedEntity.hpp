#pragma once

#include <memory>
#include <span>
#include <vector>

#include "../Node.hpp"
#include "renderer/backends/ITexture.hpp"
#include "renderer/geometry/Mesh.hpp"
#include "util/Matrix4.hpp"
#include "util/Vector.hpp"

struct alignas(16) InstanceData {
    Matrix4 modelMatrix; // 64 bytes (locations 4, 5, 6, 7)
    Vector4 color;       // 16 bytes (location 8)
};

class InstancedEntity : public Node {
public:
    auto* setMesh(this auto& self, std::shared_ptr<Mesh> newMesh) {
        self.mesh = std::move(newMesh);
        return &self;
    }

    auto* setTexture(this auto& self, std::shared_ptr<ITexture> newTex) {
        self.texture = std::move(newTex);
        return &self;
    }

    auto* setInstances(this auto& self, std::span<const InstanceData> data) {
        self.instances.assign(data.begin(), data.end());
        return &self;
    }

    auto* setInstances(this auto& self, std::vector<InstanceData> data) {
        self.instances = std::move(data);
        return &self;
    }

    [[nodiscard]] std::vector<InstanceData>& getInstancesMutable() { return instances; }

    [[nodiscard]] const std::shared_ptr<Mesh>&     getMesh() const { return mesh; }
    [[nodiscard]] const std::shared_ptr<ITexture>& getTexture() const { return texture; }
    [[nodiscard]] const std::vector<InstanceData>& getInstances() const { return instances; }
    [[nodiscard]] std::size_t                      getInstanceCount() const { return instances.size(); }

private:
    std::shared_ptr<Mesh>     mesh    = nullptr;
    std::shared_ptr<ITexture> texture = nullptr;
    std::vector<InstanceData> instances;
};
