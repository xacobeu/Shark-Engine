#include "renderer/Renderer.hpp"

#include <algorithm>

#include "renderer/geometry/Vertex.hpp"
#include "util/Matrix4.hpp"

void Renderer::init(IRenderDevice* renderDevice) {
    device = renderDevice;
    if (!defaultShader) {
        defaultShader = device->createShader("res/shaders/default.vert.glsl", "res/shaders/default.frag.glsl");
    }
    if (!instancedShader) {
        instancedShader = device->createShader("res/shaders/instanced.vert.glsl", "res/shaders/instanced.frag.glsl");
    }
}

void Renderer::begin(int width, int height) {
    // Drop GPU buffers for instanced entities that weren't drawn last frame (removed, hidden, or scene switched)
    ++frameIndex;
    std::erase_if(instanceCache, [this](const auto& entry) { return entry.second.lastUsedFrame + 1 < frameIndex; });

    if (device != nullptr) {
        device->setViewport(0, 0, width, height);
        device->begin(0.0f, 0.0f, 0.0f, 0.0f);
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
    currentViewProj    = viewProj;

    // Render standard individual entities (uninstanced)
    for (const auto& entity : scene.getEntities()) {
        if (!entity || !entity->getMesh()) {
            continue;
        }

        const auto& buffers = uploadMeshIfNeeded(entity->getMesh());

        const Transform& transform = entity->getTransform();
        Matrix4          model     = Matrix4::trs(transform.position, transform.rotationDeg, transform.scale);

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

    // Render instanced entities (opt-in 1-draw-call per group)
    for (const auto& instancedEntity : scene.getInstancedEntities()) {
        if (!instancedEntity || !instancedEntity->getMesh() || instancedEntity->getInstanceCount() == 0) {
            continue;
        }

        drawInstanced(
            instancedEntity->getMesh(), uploadInstancesIfChanged(*instancedEntity), instancedEntity->getInstanceCount(),
            instancedEntity->getTexture()
        );
    }
}

void Renderer::drawInstanced(
    const std::shared_ptr<Mesh>& mesh, std::span<const InstanceData> instances, const std::shared_ptr<ITexture>& texture
) {
    if (device == nullptr || instances.empty()) {
        return;
    }

    if (!scratchInstances) {
        scratchInstances = device->createInstanceBuffer();
    }
    scratchInstances->setData(instances.data(), instances.size_bytes());
    drawInstanced(mesh, *scratchInstances, instances.size(), texture);
}

void Renderer::drawInstanced(
    const std::shared_ptr<Mesh>& mesh, IInstanceBuffer& instances, std::size_t instanceCount,
    const std::shared_ptr<ITexture>& texture
) {
    if (device == nullptr || !mesh || instanceCount == 0 || !instancedShader) {
        return;
    }

    const auto& buffers = uploadMeshIfNeeded(mesh);

    bool hasTexture = (texture != nullptr);
    if (hasTexture) {
        texture->bind(0);
    }

    instancedShader->bind();
    instancedShader->setInt("u_HasTexture", hasTexture ? 1 : 0);
    instancedShader->setInt("u_Texture", 0);
    instancedShader->setMat4("u_ViewProjection", currentViewProj.data());

    buffers.vbo->bind();
    buffers.ebo->bind();

    device->drawIndexedInstanced(
        static_cast<unsigned int>(buffers.ebo->getIndexCount()), static_cast<unsigned int>(instanceCount), instances
    );

    if (hasTexture) {
        texture->unbind();
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

IInstanceBuffer& Renderer::uploadInstancesIfChanged(const InstancedEntity& entity) {
    InstanceBuffers& cached = instanceCache[&entity];
    cached.lastUsedFrame    = frameIndex;

    if (!cached.buffer) {
        cached.buffer = device->createInstanceBuffer();
    }
    if (cached.version != entity.getInstancesVersion()) {
        const auto& instances = entity.getInstances();
        cached.buffer->setData(instances.data(), instances.size() * sizeof(InstanceData));
        cached.version = entity.getInstancesVersion();
    }
    return *cached.buffer;
}
