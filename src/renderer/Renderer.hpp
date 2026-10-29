#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "Layer.hpp"
#include "renderer/backends/IIndexBuffer.hpp"
#include "renderer/backends/IInstanceBuffer.hpp"
#include "renderer/backends/IRenderDevice.hpp"
#include "renderer/backends/IShader.hpp"
#include "renderer/backends/IVertexBuffer.hpp"
#include "renderer/geometry/Mesh.hpp"
#include "renderer/scene/Scene.hpp"

class Renderer {
public:
    Renderer()  = default;
    ~Renderer() = default;

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) noexcept        = default;
    Renderer& operator=(Renderer&&)      = default;

    void init(IRenderDevice* device);
    void begin(int width, int height);

    void render(const Scene& scene, int width, int height);

    void drawInstanced(
        const std::shared_ptr<Mesh>& mesh, std::span<const InstanceData> instances,
        const std::shared_ptr<ITexture>& texture = nullptr
    );
    void drawInstanced(
        const std::shared_ptr<Mesh>& mesh, IInstanceBuffer& instances, std::size_t instanceCount,
        const std::shared_ptr<ITexture>& texture = nullptr
    );

    void clearCache() {
        meshCache.clear();
        instanceCache.clear();
    }

private:
    struct MeshBuffers {
        std::shared_ptr<IVertexBuffer> vbo;
        std::shared_ptr<IIndexBuffer>  ebo;
    };

    struct InstanceBuffers {
        std::shared_ptr<IInstanceBuffer> buffer;
        std::uint64_t                    version       = 0;
        std::uint64_t                    lastUsedFrame = 0;
    };

    const MeshBuffers& uploadMeshIfNeeded(const std::shared_ptr<Mesh>& mesh);
    IInstanceBuffer&   uploadInstancesIfChanged(const InstancedEntity& entity);

    IRenderDevice*                               device = nullptr;
    std::shared_ptr<IShader>                     defaultShader;
    std::shared_ptr<IShader>                     instancedShader;
    Matrix4                                      currentViewProj = Matrix4::identity();
    std::unordered_map<const Mesh*, MeshBuffers> meshCache;

    std::unordered_map<const InstancedEntity*, InstanceBuffers> instanceCache;
    std::shared_ptr<IInstanceBuffer>                            scratchInstances;
    std::uint64_t                                               frameIndex = 0;
};
