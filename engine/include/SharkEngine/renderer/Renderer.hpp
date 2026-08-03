#pragma once

#include <memory>
#include <unordered_map>
#include <SharkEngine/Layer.hpp>
#include <SharkEngine/renderer/scene/Scene.hpp>
#include <SharkEngine/renderer/backends/IRenderDevice.hpp>
#include <SharkEngine/renderer/backends/IVertexBuffer.hpp>
#include <SharkEngine/renderer/backends/IIndexBuffer.hpp>
#include <SharkEngine/renderer/backends/IShader.hpp>
#include <SharkEngine/renderer/backends/ITexture.hpp>
#include <SharkEngine/renderer/geometry/Mesh.hpp>
#include <SharkEngine/renderer/geometry/MeshFactory.hpp>
#include <SharkEngine/util/Matrix4.hpp>
#include <SharkEngine/util/Vector.hpp>

class Renderer {
public:
  Renderer() = default;
  ~Renderer() = default;

  void init(IRenderDevice* device);
  void begin(int width, int height);

  void render(const Scene &scene, int width, int height);

private:
  struct MeshBuffers {
    std::shared_ptr<IVertexBuffer> vbo;
    std::shared_ptr<IIndexBuffer> ebo;
  };

  const MeshBuffers &uploadMeshIfNeeded(const std::shared_ptr<Mesh> &mesh);

  IRenderDevice* device = nullptr;
  std::shared_ptr<IShader> defaultShader;
  std::unordered_map<const Mesh *, MeshBuffers> meshCache;
};
