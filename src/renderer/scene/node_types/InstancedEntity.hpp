#pragma once

#include <atomic>
#include <cstdint>
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
        self.markInstancesChanged();
        return &self;
    }

    auto* setInstances(this auto& self, std::vector<InstanceData> data) {
        self.instances = std::move(data);
        self.markInstancesChanged();
        return &self;
    }

    // Marks the instances changed, so writing through this each frame avoids the copy setInstances makes.
    [[nodiscard]] std::vector<InstanceData>& getInstancesMutable() {
        markInstancesChanged();
        return instances;
    }

    // Only needed when writing through a reference kept from an earlier getInstancesMutable() call.
    void markInstancesChanged() noexcept { version = ++versionCounter; }

    // The renderer re-uploads instance data only when this changes.
    [[nodiscard]] std::uint64_t getInstancesVersion() const noexcept { return version; }

    [[nodiscard]] const std::shared_ptr<Mesh>&     getMesh() const { return mesh; }
    [[nodiscard]] const std::shared_ptr<ITexture>& getTexture() const { return texture; }
    [[nodiscard]] const std::vector<InstanceData>& getInstances() const { return instances; }
    [[nodiscard]] std::size_t                      getInstanceCount() const { return instances.size(); }

private:
    std::shared_ptr<Mesh>     mesh    = nullptr;
    std::shared_ptr<ITexture> texture = nullptr;
    std::vector<InstanceData> instances;

    // Shared across entities so a cache keyed by a reused address can't mistake a new entity for an old one.
    static inline std::atomic<std::uint64_t> versionCounter{0};
    std::uint64_t                            version = 0;
};
