#pragma once
#include "IVertexBuffer.hpp"
#include "IIndexBuffer.hpp"
#include "IShader.hpp"
#include "ITexture.hpp"
#include <memory>
#include <string>

enum class GraphicsAPI {
    OpenGL
};

class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    virtual std::shared_ptr<IVertexBuffer> createVertexBuffer(const void* data, size_t size) = 0;
    virtual std::shared_ptr<IIndexBuffer> createIndexBuffer(const void* data, size_t count) = 0;
    virtual std::shared_ptr<IShader> createShader(const std::string& vertexPath, const std::string& fragmentPath) = 0;
    virtual std::shared_ptr<IShader> createShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) = 0;
    virtual std::shared_ptr<ITexture> createTexture(const std::string& filepath) = 0;

    virtual void begin(float r, float g, float b, float a) = 0;
    virtual void setViewport(int x, int y, int width, int height) = 0;
    virtual void drawIndexed(unsigned int indexCount) = 0;
    virtual void drawArrays(unsigned int vertexCount) = 0;
    virtual void setVSync(bool enabled) = 0;

    static std::unique_ptr<IRenderDevice> create(GraphicsAPI api);
};
