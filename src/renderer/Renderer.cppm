export module shark:renderer.Renderer;

import std;

import :Layer;
import :renderer.backends.IIndexBuffer;
import :renderer.backends.IInstanceBuffer;
import :renderer.backends.IRenderDevice;
import :renderer.backends.IShader;
import :renderer.backends.IVertexBuffer;
import :renderer.geometry.Mesh;
import :renderer.scene.Scene;

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
