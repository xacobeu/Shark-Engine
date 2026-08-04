module;
#include <memory>
#include <string>
#include <cstddef>

export module SharkEngine:RenderDeviceAbstractions;

export enum class GraphicsAPI {
    OpenGL
};

export class IVertexBuffer {
public:
    virtual ~IVertexBuffer() = default;
    virtual void bind() = 0;
    virtual void unbind() = 0;
    virtual void setData(const void* data, size_t size) = 0;
};

export class IIndexBuffer {
public:
    virtual ~IIndexBuffer() = default;
    virtual void bind() = 0;
    virtual void unbind() = 0;
    virtual void setData(const unsigned int* data, size_t count) = 0;
    [[nodiscard]] virtual size_t getIndexCount() const = 0;
};

export class IShader {
public:
    virtual ~IShader() = default;
    virtual void bind() = 0;
    virtual void unbind() = 0;
    virtual void setMat4(const std::string& name, const float* matrixData) = 0;
    virtual void setInt(const std::string& name, int value) = 0;
    virtual void setFloat(const std::string& name, float value) = 0;
    virtual void setVec2(const std::string& name, float x, float y) = 0;
    virtual void setVec3(const std::string& name, float x, float y, float z) = 0;
    virtual void setVec4(const std::string& name, float x, float y, float z, float w) = 0;
};

export class ITexture {
public:
    virtual ~ITexture() = default;
    virtual void bind(unsigned int slot = 0) const = 0;
    virtual void unbind() const = 0;
    [[nodiscard]] virtual int getWidth() const = 0;
    [[nodiscard]] virtual int getHeight() const = 0;
};

export class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    virtual std::shared_ptr<IVertexBuffer> createVertexBuffer(const void* data, size_t size) = 0;
    virtual std::shared_ptr<IIndexBuffer> createIndexBuffer(const void* data, size_t count) = 0;
    virtual std::shared_ptr<IShader> createShader(const std::string& vertexPath, const std::string& fragmentPath) = 0;
    virtual std::shared_ptr<IShader> createShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) = 0;
    virtual std::shared_ptr<ITexture> createTexture(const std::string& filepath) = 0;

    virtual void begin(float r, float g, float b, float a) = 0;
    virtual void endFrame() = 0;
    virtual void setViewport(int x, int y, int width, int height) = 0;
    virtual void drawIndexed(unsigned int indexCount) = 0;
    virtual void drawArrays(unsigned int vertexCount) = 0;
    virtual void setVSync(bool enabled) = 0;

    static std::unique_ptr<IRenderDevice> create(GraphicsAPI api);
};
