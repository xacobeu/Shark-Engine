#pragma once

#include <memory>
#include <unordered_map>

#include "Layer.hpp"
#include "renderer/backends/IIndexBuffer.hpp"
#include "renderer/backends/IRenderDevice.hpp"
#include "renderer/backends/IShader.hpp"
#include "renderer/backends/IVertexBuffer.hpp"
#include "renderer/geometry/Mesh.hpp"
#include "renderer/scene/Scene.hpp"

class Renderer {
public:
    Renderer()  = default;
    ~Renderer() = default;

    void init(IRenderDevice* device);
    void begin(int width, int height);

    void render(const Scene& scene, int width, int height);

private:
    struct MeshBuffers {
        std::shared_ptr<IVertexBuffer> vbo;
        std::shared_ptr<IIndexBuffer>  ebo;
    };

    const MeshBuffers& uploadMeshIfNeeded(const std::shared_ptr<Mesh>& mesh);

    IRenderDevice*                               device = nullptr;
    std::shared_ptr<IShader>                     defaultShader;
    std::unordered_map<const Mesh*, MeshBuffers> meshCache;
};
