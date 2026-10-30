export module shark:renderer.Renderer;

import std;

import :Layer;
import :renderer.backends.IBuffer;
import :renderer.backends.IPipeline;
import :renderer.backends.IRenderDevice;
import :renderer.backends.ITexture;
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
    void end();

    void render(const Scene& scene, int width, int height);

    void drawInstanced(
        const std::shared_ptr<Mesh>& mesh, std::span<const InstanceData> instances,
        const std::shared_ptr<ITexture>& texture = nullptr
    );
    void drawInstanced(
        const std::shared_ptr<Mesh>& mesh, const IBuffer& instances, std::size_t instanceCount,
        const std::shared_ptr<ITexture>& texture = nullptr
    );

    inline void clearCache() {
        meshCache.clear();
        instanceCache.clear();
        scratchInstances.clear();
    }

private:
    // Binding numbers shared by the default and instanced pipelines.
    static constexpr std::uint32_t FRAME_BINDING   = 0;
    static constexpr std::uint32_t OBJECT_BINDING  = 1;
    static constexpr std::uint32_t TEXTURE_BINDING = 2;

    // std140 mirrors of the shader uniform blocks.
    struct FrameUniforms {
        Matrix4                                viewProjection = Matrix4::identity();
        Vector4                                cameraPos{0.0f, 0.0f, 0.0f, 0.0f};
        std::array<Vector4, Scene::MAX_LIGHTS> lightPosType{};
        std::array<Vector4, Scene::MAX_LIGHTS> lightColorRange{};
        std::int32_t                           lightCount = 0;
        std::array<std::int32_t, 3>            padding{};
    };
    static_assert(sizeof(FrameUniforms) == 64 + 16 + (Scene::MAX_LIGHTS * 32) + 16);

    struct ObjectUniforms {
        Matrix4      model;
        Matrix4      normalMatrix;
        Vector4      color;
        float        specular   = Entity::DEFAULT_SPECULAR;
        float        shininess  = Entity::DEFAULT_SHININESS;
        std::int32_t hasTexture = 0;
        std::int32_t padding    = 0;
    };
    static_assert(sizeof(ObjectUniforms) == 160);

    struct MaterialUniforms {
        float        specular   = Entity::DEFAULT_SPECULAR;
        float        shininess  = Entity::DEFAULT_SHININESS;
        std::int32_t hasTexture = 0;
        std::int32_t padding    = 0;
    };
    static_assert(sizeof(MaterialUniforms) == 16);

    struct MeshBuffers {
        std::shared_ptr<IBuffer> vertices;
        std::shared_ptr<IBuffer> indices;
        std::uint32_t            indexCount = 0;
    };

    struct InstanceBuffers {
        std::shared_ptr<IBuffer> buffer;
        std::uint64_t            version       = 0;
        std::uint64_t            lastUsedFrame = 0;
    };

    const MeshBuffers& uploadMeshIfNeeded(const std::shared_ptr<Mesh>& mesh);
    const IBuffer&     uploadInstancesIfChanged(const InstancedEntity& entity);
    void               packLights(std::span<const std::shared_ptr<Light>> lights, const Vector3& cameraPos);

    IRenderDevice*                               device = nullptr;
    std::shared_ptr<IPipeline>                   defaultPipeline;
    std::shared_ptr<IPipeline>                   instancedPipeline;
    FrameUniforms                                frameUniforms;
    std::unordered_map<const Mesh*, MeshBuffers> meshCache;

    std::unordered_map<const InstancedEntity*, InstanceBuffers> instanceCache;
    // One buffer per drawInstanced(span) call this frame, since a buffer is written at most once per frame.
    std::vector<std::shared_ptr<IBuffer>> scratchInstances;
    std::size_t                           scratchUsed = 0;
    std::uint64_t                         frameIndex  = 0;
};

} // namespace shark

namespace shark {

void Renderer::init(IRenderDevice* renderDevice) {
    device = renderDevice;

    const BindingDesc frameBinding{.binding = FRAME_BINDING, .type = BindingType::UniformBuffer, .name = "FrameData"};
    const BindingDesc textureBinding{.binding = TEXTURE_BINDING, .type = BindingType::Texture, .name = "u_Texture"};

    if (!defaultPipeline) {
        defaultPipeline = device->createPipeline({
            .shader        = "res/shaders/default",
            .vertexBuffers = {Vertex::layout()},
            .bindings      = {
                frameBinding,
                {.binding = OBJECT_BINDING, .type = BindingType::UniformBuffer, .name = "ObjectData"},
                textureBinding,
            },
        });
    }
    if (!instancedPipeline) {
        instancedPipeline = device->createPipeline({
            .shader        = "res/shaders/instanced",
            .vertexBuffers = {Vertex::layout(), InstanceData::layout()},
            .bindings      = {
                frameBinding,
                {.binding = OBJECT_BINDING, .type = BindingType::UniformBuffer, .name = "MaterialData"},
                textureBinding,
            },
        });
    }

    packLights({}, {0.0f, 0.0f, 0.0f});
}

void Renderer::begin(int width, int height) {
    // Drop GPU buffers for instanced entities that weren't drawn last frame (removed, hidden, or scene switched)
    ++frameIndex;
    std::erase_if(instanceCache, [this](const auto& entry) { return entry.second.lastUsedFrame + 1 < frameIndex; });
    scratchUsed = 0;

    if (device != nullptr) {
        device->beginFrame(width, height, {0.0f, 0.0f, 0.0f, 0.0f});
    }
}

void Renderer::end() {
    if (device != nullptr) {
        device->endFrame();
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

    Matrix4 projection           = camera->getProjectionMatrix(aspect);
    Matrix4 view                 = camera->getViewMatrix();
    frameUniforms.viewProjection = projection * view;

    packLights(scene.getLights(), camera->getEyePosition());

    // Render standard individual entities (uninstanced)
    device->setPipeline(*defaultPipeline);
    device->setUniforms(FRAME_BINDING, frameUniforms);
    for (const auto& entity : scene.getEntities()) {
        if (!entity || !entity->getMesh()) {
            continue;
        }

        const auto& buffers = uploadMeshIfNeeded(entity->getMesh());

        const Transform& transform = entity->getTransform();

        auto    safeInverse  = [](float s) { return (s != 0.0f) ? 1.0f / s : 0.0f; };
        Vector3 inverseScale = {
            safeInverse(transform.scale.x),
            safeInverse(transform.scale.y),
            safeInverse(transform.scale.z),
        };

        const ObjectUniforms object{
            .model        = Matrix4::trs(transform.position, transform.rotationDeg, transform.scale),
            .normalMatrix = Matrix4::trs({0.0f, 0.0f, 0.0f}, transform.rotationDeg, inverseScale),
            .color        = entity->getColor(),
            .specular     = entity->getSpecular(),
            .shininess    = entity->getShininess(),
            .hasTexture   = entity->getTexture() != nullptr ? 1 : 0,
        };
        device->setUniforms(OBJECT_BINDING, object);
        device->setTexture(TEXTURE_BINDING, entity->getTexture().get());

        device->setVertexBuffer(0, *buffers.vertices);
        device->setIndexBuffer(*buffers.indices);
        device->drawIndexed(buffers.indexCount);
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

    if (scratchUsed == scratchInstances.size()) {
        scratchInstances.push_back(device->createBuffer(BufferUsage::Vertex, nullptr, 0));
    }
    IBuffer& scratch = *scratchInstances[scratchUsed++];
    scratch.setData(instances.data(), instances.size_bytes());
    drawInstanced(mesh, scratch, instances.size(), texture);
}

void Renderer::drawInstanced(
    const std::shared_ptr<Mesh>& mesh, const IBuffer& instances, std::size_t instanceCount,
    const std::shared_ptr<ITexture>& texture
) {
    if (device == nullptr || !mesh || instanceCount == 0 || !instancedPipeline) {
        return;
    }

    const auto& buffers = uploadMeshIfNeeded(mesh);

    // Frame data is re-sent because a layer's own pipeline may have reused its binding since render().
    device->setPipeline(*instancedPipeline);
    device->setUniforms(FRAME_BINDING, frameUniforms);
    device->setUniforms(OBJECT_BINDING, MaterialUniforms{.hasTexture = texture != nullptr ? 1 : 0});
    device->setTexture(TEXTURE_BINDING, texture.get());

    device->setVertexBuffer(0, *buffers.vertices);
    device->setVertexBuffer(1, instances);
    device->setIndexBuffer(*buffers.indices);
    device->drawIndexed(buffers.indexCount, static_cast<std::uint32_t>(instanceCount));
}

void Renderer::packLights(std::span<const std::shared_ptr<Light>> lights, const Vector3& cameraPos) {
    frameUniforms.cameraPos       = {cameraPos.x, cameraPos.y, cameraPos.z, 0.0f};
    frameUniforms.lightPosType    = {};
    frameUniforms.lightColorRange = {};
    frameUniforms.lightCount      = 0;

    auto push = [this](Vector3 posOrDir, LightType type, Vector4 color, float intensity, float range) {
        const auto index                     = static_cast<std::size_t>(frameUniforms.lightCount);
        frameUniforms.lightPosType[index]    = {posOrDir.x, posOrDir.y, posOrDir.z, static_cast<float>(type)};
        frameUniforms.lightColorRange[index] = {color.x * intensity, color.y * intensity, color.z * intensity, range};
        ++frameUniforms.lightCount;
    };

    if (lights.empty()) {
        const Vector4 white{1.0f, 1.0f, 1.0f, 1.0f};
        push({0.0f, 0.0f, 0.0f}, LightType::Ambient, white, 0.3f, 0.0f);
        push(normalize(Vector3{-1.0f, -1.0f, -1.0f}), LightType::Directional, white, 1.0f, 0.0f);
        return;
    }

    for (const auto& light : lights) {
        if (!light || std::cmp_greater_equal(frameUniforms.lightCount, Scene::MAX_LIGHTS)) {
            continue;
        }
        const Vector3 posOrDir = (light->getType() == LightType::Directional) ? light->getDirection()
                                                                              : light->getPosition();
        push(posOrDir, light->getType(), light->getColor(), light->getIntensity(), light->getRange());
    }
}

const Renderer::MeshBuffers& Renderer::uploadMeshIfNeeded(const std::shared_ptr<Mesh>& mesh) {
    const Mesh* key = mesh.get();
    auto        it  = meshCache.find(key);
    if (it != meshCache.end()) {
        return it->second;
    }

    const auto& meshData = mesh->getData();

    MeshBuffers buffers{
        .vertices   = device->createBuffer(BufferUsage::Vertex, std::span{meshData.vertices}),
        .indices    = device->createBuffer(BufferUsage::Index, std::span{meshData.triangles}),
        .indexCount = static_cast<std::uint32_t>(meshData.triangles.size()),
    };

    auto [insertedIt, _] = meshCache.emplace(key, std::move(buffers));
    return insertedIt->second;
}

const IBuffer& Renderer::uploadInstancesIfChanged(const InstancedEntity& entity) {
    InstanceBuffers& cached = instanceCache[&entity];
    cached.lastUsedFrame    = frameIndex;

    if (!cached.buffer) {
        cached.buffer = device->createBuffer(BufferUsage::Vertex, nullptr, 0);
    }
    if (cached.version != entity.getInstancesVersion()) {
        const auto& instances = entity.getInstances();
        cached.buffer->setData(instances.data(), instances.size() * sizeof(InstanceData));
        cached.version = entity.getInstancesVersion();
    }
    return *cached.buffer;
}

} // namespace shark
