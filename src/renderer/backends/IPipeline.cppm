export module shark:renderer.backends.IPipeline;

import std;

export namespace shark {
enum class VertexFormat : std::uint8_t { Float, Float2, Float3, Float4 };
enum class VertexStepMode : std::uint8_t { Vertex, Instance };

struct VertexAttribute {
    std::uint32_t location = 0;
    VertexFormat  format   = VertexFormat::Float4;
    std::uint32_t offset   = 0;
};

// One entry per vertex buffer slot passed to IRenderDevice::setVertexBuffer.
struct VertexBufferLayout {
    std::uint32_t                stride   = 0;
    VertexStepMode               stepMode = VertexStepMode::Vertex;
    std::vector<VertexAttribute> attributes;
};

enum class BindingType : std::uint8_t { UniformBuffer, Texture };

// Binding numbers are what the device's setUniforms/setTexture take. Backends without explicit binding
// numbers in their shading language (GLSL 330 / ESSL 300) find the block or sampler by name instead.
struct BindingDesc {
    std::uint32_t binding = 0;
    BindingType   type    = BindingType::UniformBuffer;
    std::string   name;
};

struct PipelineDesc {
    // Path without extension; each backend appends its own (".vert.glsl" and ".frag.glsl" for OpenGL).
    std::string                     shader;
    std::vector<VertexBufferLayout> vertexBuffers;
    std::vector<BindingDesc>        bindings;
    bool                            depthTest  = true;
    bool                            depthWrite = true;
};

class IPipeline {
public:
    virtual ~IPipeline() = default;

    IPipeline(const IPipeline&)            = delete ("GPU resources are shared via std::shared_ptr");
    IPipeline& operator=(const IPipeline&) = delete ("GPU resources are shared via std::shared_ptr");

    IPipeline(IPipeline&&)            = default;
    IPipeline& operator=(IPipeline&&) = default;

protected:
    IPipeline() = default;
};

} // namespace shark
