export module shark:renderer.Renderer;

import std;

import :Layer;
import :renderer.backends.IIndexBuffer;
import :renderer.backends.IInstanceBuffer;
import :renderer.backends.IRenderDevice;
import :renderer.backends.IShader;
import :renderer.backends.ITexture;
import :renderer.backends.IVertexBuffer;
import :renderer.geometry.Mesh;
import :renderer.geometry.Vertex;
import :renderer.scene.Node;
import :renderer.scene.Scene;
import :renderer.scene.node_types.Camera;
import :renderer.scene.node_types.Entity;
import :renderer.scene.node_types.InstancedEntity;
import :renderer.scene.node_types.Light;
import :util.Matrix4;
import :util.Vector;

export namespace shark {
class Renderer {
public:
    Renderer()  = default;
    ~Renderer() = default;

    Renderer(const Renderer&)            = delete ("owns GPU caches; pass it by reference");
    Renderer& operator=(const Renderer&) = delete ("owns GPU caches; pass it by reference");
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

    inline void clearCache() {
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

    struct LightBlock {
        std::array<float, Scene::MAX_LIGHTS * 4> posType{};
        std::array<float, Scene::MAX_LIGHTS * 4> colorRange{};
        int                                      count = 0;
        Vector3                                  cameraPos{0.0f, 0.0f, 0.0f};
    };

    const MeshBuffers& uploadMeshIfNeeded(const std::shared_ptr<Mesh>& mesh);
    IInstanceBuffer&   uploadInstancesIfChanged(const InstancedEntity& entity);
    void               packLights(std::span<const std::shared_ptr<Light>> lights, const Vector3& cameraPos);
    void               applyLighting(IShader& shader) const;

    IRenderDevice*                               device = nullptr;
    std::shared_ptr<IShader>                     defaultShader;
    std::shared_ptr<IShader>                     instancedShader;
    Matrix4                                      currentViewProj = Matrix4::identity();
    LightBlock                                   lightBlock;
    std::unordered_map<const Mesh*, MeshBuffers> meshCache;

    std::unordered_map<const InstancedEntity*, InstanceBuffers> instanceCache;
    std::shared_ptr<IInstanceBuffer>                            scratchInstances;
    std::uint64_t                                               frameIndex = 0;
};

} // namespace shark

namespace shark {

void Renderer::init(IRenderDevice* renderDevice) {
    device = renderDevice;
    if (!defaultShader) {
        defaultShader = device->createShader("res/shaders/default.vert.glsl", "res/shaders/default.frag.glsl");
    }
    if (!instancedShader) {
        instancedShader = device->createShader("res/shaders/instanced.vert.glsl", "res/shaders/instanced.frag.glsl");
    }

    packLights({}, {0.0f, 0.0f, 0.0f});
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

    packLights(scene.getLights(), camera->getEyePosition());

    // Render standard individual entities (uninstanced)
    for (const auto& entity : scene.getEntities()) {
        if (!entity || !entity->getMesh()) {
            continue;
        }

        const auto& buffers = uploadMeshIfNeeded(entity->getMesh());

        const Transform& transform = entity->getTransform();
        Matrix4          model     = Matrix4::trs(transform.position, transform.rotationDeg, transform.scale);

        auto    safeInverse  = [](float s) { return (s != 0.0f) ? 1.0f / s : 0.0f; };
        Vector3 inverseScale = {
            safeInverse(transform.scale.x),
            safeInverse(transform.scale.y),
            safeInverse(transform.scale.z),
        };
        Matrix4 normalMatrix = Matrix4::trs({0.0f, 0.0f, 0.0f}, transform.rotationDeg, inverseScale);

        bool hasTexture = (entity->getTexture() != nullptr);
        if (hasTexture) {
            entity->getTexture()->bind(0);
        }

        defaultShader->bind();
        defaultShader->setInt("u_HasTexture", hasTexture ? 1 : 0);
        defaultShader->setInt("u_Texture", 0);
        Vector4 color = entity->getColor();
        defaultShader->setVec4("u_Color", color.x, color.y, color.z, color.w);
        defaultShader->setFloat("u_Specular", entity->getSpecular());
        defaultShader->setFloat("u_Shininess", entity->getShininess());
        defaultShader->setMat4("u_Model", model.data());
        defaultShader->setMat4("u_NormalMatrix", normalMatrix.data());
        defaultShader->setMat4("u_ViewProjection", viewProj.data());
        applyLighting(*defaultShader);

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
    instancedShader->setFloat("u_Specular", Entity::DEFAULT_SPECULAR);
    instancedShader->setFloat("u_Shininess", Entity::DEFAULT_SHININESS);
    applyLighting(*instancedShader);

    buffers.vbo->bind();
    buffers.ebo->bind();

    device->drawIndexedInstanced(
        static_cast<unsigned int>(buffers.ebo->getIndexCount()), static_cast<unsigned int>(instanceCount), instances
    );

    if (hasTexture) {
        texture->unbind();
    }
}

void Renderer::packLights(std::span<const std::shared_ptr<Light>> lights, const Vector3& cameraPos) {
    lightBlock           = {};
    lightBlock.cameraPos = cameraPos;

    auto push = [this](Vector3 posOrDir, LightType type, Vector4 color, float intensity, float range) {
        const auto base                 = static_cast<std::size_t>(lightBlock.count) * 4;
        lightBlock.posType[base + 0]    = posOrDir.x;
        lightBlock.posType[base + 1]    = posOrDir.y;
        lightBlock.posType[base + 2]    = posOrDir.z;
        lightBlock.posType[base + 3]    = static_cast<float>(type);
        lightBlock.colorRange[base + 0] = color.x * intensity;
        lightBlock.colorRange[base + 1] = color.y * intensity;
        lightBlock.colorRange[base + 2] = color.z * intensity;
        lightBlock.colorRange[base + 3] = range;
        ++lightBlock.count;
    };

    if (lights.empty()) {
        const Vector4 white{1.0f, 1.0f, 1.0f, 1.0f};
        push({0.0f, 0.0f, 0.0f}, LightType::Ambient, white, 0.3f, 0.0f);
        push(normalize(Vector3{-1.0f, -1.0f, -1.0f}), LightType::Directional, white, 1.0f, 0.0f);
        return;
    }

    for (const auto& light : lights) {
        if (!light || std::cmp_greater_equal(lightBlock.count, Scene::MAX_LIGHTS)) {
            continue;
        }
        const Vector3 posOrDir = (light->getType() == LightType::Directional) ? light->getDirection()
                                                                              : light->getPosition();
        push(posOrDir, light->getType(), light->getColor(), light->getIntensity(), light->getRange());
    }
}

void Renderer::applyLighting(IShader& shader) const {
    shader.setInt("u_LightCount", lightBlock.count);
    const auto count = static_cast<std::size_t>(lightBlock.count);
    shader.setVec4Array("u_LightPosType[0]", lightBlock.posType.data(), count);
    shader.setVec4Array("u_LightColorRange[0]", lightBlock.colorRange.data(), count);
    shader.setVec3("u_CameraPos", lightBlock.cameraPos.x, lightBlock.cameraPos.y, lightBlock.cameraPos.z);
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

    auto [insertedIt, _] = meshCache.emplace(key, std::move(buffers));
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

} // namespace shark
