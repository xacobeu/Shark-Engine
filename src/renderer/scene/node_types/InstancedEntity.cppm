export module shark:renderer.scene.node_types.InstancedEntity;

import std;

import :renderer.scene.Node;
import :renderer.backends.IPipeline;
import :renderer.backends.ITexture;
import :renderer.geometry.Mesh;
import :util.Matrix4;
import :util.Vector;

export namespace shark {
struct alignas(16) InstanceData {
    Matrix4 modelMatrix; // 64 bytes (locations 4, 5, 6, 7)
    Vector4 color;       // 16 bytes (location 8)

    [[nodiscard]] static VertexBufferLayout layout() {
        constexpr std::uint32_t column = sizeof(float) * 4;
        return {
            .stride     = sizeof(InstanceData),
            .stepMode   = VertexStepMode::Instance,
            .attributes = {
                {.location = 4, .format = VertexFormat::Float4, .offset = 0 * column},
                {.location = 5, .format = VertexFormat::Float4, .offset = 1 * column},
                {.location = 6, .format = VertexFormat::Float4, .offset = 2 * column},
                {.location = 7, .format = VertexFormat::Float4, .offset = 3 * column},
                {.location = 8, .format = VertexFormat::Float4, .offset = sizeof(Matrix4)},
            },
        };
    }
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
    [[nodiscard]] inline std::vector<InstanceData>& getInstancesMutable() {
        markInstancesChanged();
        return instances;
    }

    // Only needed when writing through a reference kept from an earlier getInstancesMutable() call.
    inline void markInstancesChanged() noexcept { version = ++versionCounter; }

    // The renderer re-uploads instance data only when this changes.
    [[nodiscard]] inline std::uint64_t getInstancesVersion() const noexcept { return version; }

    [[nodiscard]] inline const std::shared_ptr<Mesh>&     getMesh() const { return mesh; }
    [[nodiscard]] inline const std::shared_ptr<ITexture>& getTexture() const { return texture; }
    [[nodiscard]] inline const std::vector<InstanceData>& getInstances() const { return instances; }
    [[nodiscard]] inline std::size_t                      getInstanceCount() const { return instances.size(); }

private:
    std::shared_ptr<Mesh>     mesh    = nullptr;
    std::shared_ptr<ITexture> texture = nullptr;
    std::vector<InstanceData> instances;

    // Shared across entities so a cache keyed by a reused address can't mistake a new entity for an old one.
    static inline std::atomic<std::uint64_t> versionCounter{0};
    std::uint64_t                            version = 0;
};

} // namespace shark
