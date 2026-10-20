#include "renderer/Renderer.hpp"

#include <algorithm>

#include "renderer/geometry/Vertex.hpp"
#include "util/Matrix4.hpp"

void Renderer::init(IRenderDevice* renderDevice) {
    device = renderDevice;
    if (!defaultShader) {
        defaultShader = device->createShader("res/shaders/default.vert.glsl", "res/shaders/default.frag.glsl");
    }
}

void Renderer::begin(int width, int height) {
    if (device != nullptr) {
        device->setViewport(0, 0, width, height);
        device->begin(0.05f, 0.2f, 0.35f, 1.0f);
    }
}

void Renderer::render(const Scene& scene, int width, int height) {
    if (device == nullptr) {
        return;
    }

    Camera* camera = scene.getActiveCamera();
    if (camera == nullptr) {
        return;
    }

    auto safeHeight = static_cast<float>(height);
    safeHeight      = std::max(safeHeight, 1.0f);
    float aspect    = static_cast<float>(width) / safeHeight;

    Matrix4 projection = camera->getProjectionMatrix(aspect);
    Matrix4 view       = camera->getViewMatrix();
    Matrix4 viewProj   = projection * view;

    for (const auto& entity : scene.getEntities()) {
        if (!entity || !entity->getMesh()) {
            continue;
        }

        const auto& buffers = uploadMeshIfNeeded(entity->getMesh());

        const Transform& transform = entity->getTransform();
        Matrix4          model = Matrix4::translate(transform.position) * Matrix4::rotateZ(transform.rotationDeg.z) *
                                 Matrix4::rotateY(transform.rotationDeg.y) * Matrix4::rotateX(transform.rotationDeg.x) *
                                 Matrix4::scale(transform.scale);

        Matrix4 mvp = viewProj * model;

        bool hasTexture = (entity->getTexture() != nullptr);
        if (hasTexture) {
            entity->getTexture()->bind(0);
        }

        defaultShader->bind();
        defaultShader->setInt("u_HasTexture", hasTexture ? 1 : 0);
        defaultShader->setInt("u_Texture", 0);
        Vector4 color = entity->getColor();
        defaultShader->setVec4("u_Color", color.x, color.y, color.z, color.w);
        defaultShader->setMat4("u_Transform", mvp.data());

        buffers.vbo->bind();
        buffers.ebo->bind();
        device->drawIndexed(static_cast<unsigned int>(buffers.ebo->getIndexCount()));

        if (hasTexture) {
            entity->getTexture()->unbind();
        }
    }
}

const Renderer::MeshBuffers& Renderer::uploadMeshIfNeeded(const std::shared_ptr<Mesh>& mesh) {
    const Mesh* key = mesh.get();
    auto        it  = meshCache.find(key);
    if (it != meshCache.end()) {
        return it->second;
    }

    const auto& meshData = mesh->getData();

    MeshBuffers buffers;
    buffers.vbo = device->createVertexBuffer(meshData.vertices.data(), meshData.vertices.size() * sizeof(Vertex));
    buffers.ebo = device->createIndexBuffer(meshData.triangles.data(), meshData.triangles.size());

    auto [insertedIt, inserted] = meshCache.emplace(key, std::move(buffers));
    (void)inserted;
    return insertedIt->second;
}
