#include <SharkEngine/renderer/Renderer.hpp>
#include <SharkEngine/util/Matrix4.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void Renderer::init() {
  if (!defaultShader) {
    defaultShader = std::make_shared<GL::Shader>();
    defaultShader->compileFromFiles("res/shaders/vertex.glsl",
                                    "res/shaders/fragment.glsl");
  }
}

void Renderer::begin(int width, int height) {
  glViewport(0, 0, width, height);
  glClearColor(0.05f, 0.2f, 0.35f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
}

void Renderer::beginScene(const Camera &camera, int width, int height,
                          float fov) {
  init();

  float safeHeight = static_cast<float>(height);
  if (safeHeight < 1.0f)
    safeHeight = 1.0f;
  float aspect = static_cast<float>(width) / safeHeight;

  Matrix4 projection = camera.getProjectionMatrix(aspect, fov);
  Matrix4 view = camera.getViewMatrix();
  sceneViewProjection = projection * view;
  sceneBegun = true;
}

Renderer::GameObject Renderer::createGameObject(const Mesh &mesh) const {
  GameObject object;
  object.mesh = std::make_shared<Mesh>(mesh);
  return object;
}

Renderer::GameObject Renderer::createGameObject(Mesh &&mesh) const {
  GameObject object;
  object.mesh = std::make_shared<Mesh>(std::move(mesh));
  return object;
}

const Renderer::MeshBuffers &
Renderer::uploadMeshIfNeeded(const std::shared_ptr<Mesh> &mesh) {
  const Mesh *key = mesh.get();
  auto it = meshCache.find(key);
  if (it != meshCache.end()) {
    return it->second;
  }

  MeshBuffers buffers;
  buffers.vao = std::make_shared<GL::VertexArray>();
  buffers.vbo = std::make_shared<GL::VertexBuffer>();
  buffers.ebo = std::make_shared<GL::ElementBuffer>();
  buffers.indexCount = std::make_shared<int>(0);

  buffers.vao->create();
  buffers.vbo->create();
  buffers.ebo->create();

  const auto &meshData = mesh->getData();
  *buffers.indexCount = static_cast<int>(meshData.triangles.size());

  buffers.vao->bind();
  buffers.vbo->bind();
  buffers.vbo->data(
      static_cast<GLsizeiptr>(meshData.vertices.size() * sizeof(Vertex)),
      meshData.vertices.data(), GL_STATIC_DRAW);

  buffers.ebo->bind();
  buffers.ebo->data(
      static_cast<GLsizeiptr>(meshData.triangles.size() * sizeof(int)),
      meshData.triangles.data(), GL_STATIC_DRAW);

  buffers.vao->setLayout<Vertex>();
  buffers.vao->unbind();

  auto [insertedIt, inserted] = meshCache.emplace(key, std::move(buffers));
  (void)inserted;
  return insertedIt->second;
}

void Renderer::submit(const GameObject &object) {
  if (!sceneBegun || !object.visible || !object.mesh) {
    return;
  }

  const auto &buffers = uploadMeshIfNeeded(object.mesh);

  Matrix4 model = Matrix4::translate(object.transform.position) *
                  Matrix4::rotateZ(object.transform.rotationDeg.z) *
                  Matrix4::rotateY(object.transform.rotationDeg.y) *
                  Matrix4::rotateX(object.transform.rotationDeg.x) *
                  Matrix4::scale(object.transform.scale);

  Matrix4 mvp = sceneViewProjection * model;
  submit(buffers.vao, buffers.indexCount, defaultShader, mvp);
}

void Renderer::submit(const std::shared_ptr<GL::VertexArray> &vao,
                      const std::shared_ptr<int> indexCount,
                      const std::shared_ptr<GL::Shader> &shader,
                      const Matrix4 &transform) {

  shader->bind();
  shader->setMat4("u_Transform", transform.data());

  vao->bind();
  glDrawElements(GL_TRIANGLES, *indexCount, GL_UNSIGNED_INT, 0);
  vao->unbind();
  shader->unbind();
}
