export module shark:renderer.backends.IRenderDevice;

import std;

import :Window;
import :renderer.backends.GraphicsAPI;
import :renderer.backends.IBuffer;
import :renderer.backends.IPipeline;
import :renderer.backends.ITexture;
import :util.Vector;

export namespace shark {
// Draw state set between beginFrame and endFrame (pipeline, buffers, uniforms, textures) stays bound until it is
// replaced or the frame ends. Resources passed by reference must outlive the draws that use them.
class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    IRenderDevice(const IRenderDevice&)            = delete ("owns the graphics context; pass a pointer");
    IRenderDevice& operator=(const IRenderDevice&) = delete ("owns the graphics context; pass a pointer");
    IRenderDevice(IRenderDevice&&)                 = default;
    IRenderDevice& operator=(IRenderDevice&&)      = default;

    virtual std::shared_ptr<IBuffer> createBuffer(BufferUsage usage, const void* data, std::size_t size) = 0;
    template <typename T, std::size_t Extent = std::dynamic_extent>
    std::shared_ptr<IBuffer> createBuffer(BufferUsage usage, std::span<T, Extent> buffer) {
        return createBuffer(usage, buffer.data(), buffer.size_bytes());
    }
    virtual std::shared_ptr<ITexture>  createTexture(const std::string& filepath) = 0;
    virtual std::shared_ptr<IPipeline> createPipeline(const PipelineDesc& desc)   = 0;

    // Starts ImGui's renderer side of the frame; call before ImGui::NewFrame.
    virtual void beginImGuiFrame() = 0;

    // Clears the backbuffer and sets the viewport to cover it.
    virtual void beginFrame(int width, int height, const Vector4& clearColor) = 0;
    // Draws ImGui's last rendered draw data (if any) and presents.
    virtual void endFrame() = 0;

    virtual void setViewport(int x, int y, int width, int height)           = 0;
    virtual void setPipeline(const IPipeline& pipeline)                     = 0;
    virtual void setVertexBuffer(std::uint32_t slot, const IBuffer& buffer) = 0;
    virtual void setIndexBuffer(const IBuffer& buffer)                      = 0;
    // Copies the data, so it may change between draws. It must cover the whole uniform block (std140 layout).
    virtual void setUniformData(std::uint32_t binding, const void* data, std::size_t size) = 0;
    template <typename T>
        requires std::is_trivially_copyable_v<T>
    void setUniforms(std::uint32_t binding, const T& data) {
        setUniformData(binding, &data, sizeof(T));
    }
    virtual void setTexture(std::uint32_t binding, const ITexture* texture) = 0;

    virtual void draw(std::uint32_t vertexCount, std::uint32_t instanceCount = 1)       = 0;
    virtual void drawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount = 1) = 0;

    virtual void setVSync(bool enabled) = 0;

    [[nodiscard]] static std::unique_ptr<IRenderDevice> create(GraphicsAPI api, Window& window);

protected:
    IRenderDevice() = default;
};

} // namespace shark
