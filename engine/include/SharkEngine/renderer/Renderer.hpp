#pragma once

#ifdef SHARK_NO_GLAD
	#include <GLES3/gl3.h>
#else
	#include <glad/glad.h>
#endif

#include <memory>
#include <unordered_map>

#include <SharkEngine/Layer.hpp>
#include <SharkEngine/renderer/Camera.hpp>
#include <SharkEngine/renderer/GL/GLObject.hpp>
#include <SharkEngine/renderer/GL/Shader.hpp>
#include <SharkEngine/renderer/geometry/Mesh.hpp>
#include <SharkEngine/renderer/geometry/MeshFactory.hpp>
#include <SharkEngine/util/Matrix4.hpp>
#include <SharkEngine/util/Pi.hpp>
#include <SharkEngine/util/Vector.hpp>


class Renderer {
public:
  struct Transform {
    Vector3 position{0.0f, 0.0f, 0.0f};
    Vector3 rotationDeg{0.0f, 0.0f, 0.0f};
    Vector3 scale{1.0f, 1.0f, 1.0f};
  };

  struct GameObject {
    std::shared_ptr<Mesh> mesh;
    Transform transform;
    bool visible = true;
  };

  Renderer() = default;
  ~Renderer() = default;

  void init();
  void begin(int width, int height);

  void beginScene(const Camera &camera, int width, int height,
                  float fov = 45.0f);
  GameObject createGameObject(const Mesh &mesh) const;
  GameObject createGameObject(Mesh &&mesh) const;
  void submit(const GameObject &object);

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
  Matrix4 sceneViewProjection = Matrix4::identity();
  bool sceneBegun = false;
  std::unordered_map<const Mesh *, MeshBuffers> meshCache;
};
