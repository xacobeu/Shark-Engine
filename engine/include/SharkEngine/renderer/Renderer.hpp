#pragma once

#ifdef SHARK_NO_GLAD
	#include <GLES3/gl3.h>
#else
	#include <glad/glad.h>
#endif

#include <memory>
#include <unordered_map>

#include <SharkEngine/Layer.hpp>
#include <SharkEngine/renderer/scene/Scene.hpp>
#include <SharkEngine/renderer/GL/GLObject.hpp>
#include <SharkEngine/renderer/GL/Shader.hpp>
#include <SharkEngine/renderer/geometry/Mesh.hpp>
#include <SharkEngine/renderer/geometry/MeshFactory.hpp>
#include <SharkEngine/util/Matrix4.hpp>
#include <SharkEngine/util/Pi.hpp>
#include <SharkEngine/util/Vector.hpp>


class Renderer {
public:
  Renderer() = default;
  ~Renderer() = default;

  void init();
  void begin(int width, int height);

  void render(const Scene &scene, int width, int height);

  void submit(const std::shared_ptr<GL::VertexArray> &vao,
              const std::shared_ptr<int> indexCount,
              const std::shared_ptr<GL::Shader> &shader,
              const Matrix4 &transform);

private:
  struct MeshBuffers {
    std::shared_ptr<GL::VertexArray> vao;
    std::shared_ptr<GL::VertexBuffer> vbo;
    std::shared_ptr<GL::ElementBuffer> ebo;
    std::shared_ptr<int> indexCount;
  };

  const MeshBuffers &uploadMeshIfNeeded(const std::shared_ptr<Mesh> &mesh);

  std::shared_ptr<GL::Shader> defaultShader;
  std::unordered_map<const Mesh *, MeshBuffers> meshCache;
};
