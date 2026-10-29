#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>

#include "IIndexBuffer.hpp"
#include "IInstanceBuffer.hpp"
#include "IShader.hpp"
#include "ITexture.hpp"
#include "IVertexBuffer.hpp"

enum class GraphicsAPI : std::uint8_t { OpenGL };

class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    IRenderDevice(const IRenderDevice&)            = delete;
    IRenderDevice& operator=(const IRenderDevice&) = delete;
    IRenderDevice(IRenderDevice&&)                 = default;
    IRenderDevice& operator=(IRenderDevice&&)      = default;

    virtual std::shared_ptr<IVertexBuffer> createVertexBuffer(const void* data, std::size_t size) = 0;
    template <typename T, std::size_t Extent = std::dynamic_extent>
    std::shared_ptr<IVertexBuffer> createVertexBuffer(std::span<T, Extent> buffer) {
        return createVertexBuffer(buffer.data(), buffer.size_bytes());
    }
    virtual std::shared_ptr<IIndexBuffer> createIndexBuffer(const void* data, std::size_t count)                  = 0;
    virtual std::shared_ptr<IShader> createShader(const std::string& vertexPath, const std::string& fragmentPath) = 0;
    virtual std::shared_ptr<IShader>
    createShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) = 0;
    virtual std::shared_ptr<ITexture>        createTexture(const std::string& filepath)        = 0;
    virtual std::shared_ptr<IInstanceBuffer> createInstanceBuffer()                            = 0;

    virtual void begin(float r, float g, float b, float a)        = 0;
    virtual void setViewport(int x, int y, int width, int height) = 0;
    virtual void drawIndexed(unsigned int indexCount)             = 0;
    virtual void drawArrays(unsigned int vertexCount)             = 0;
    virtual void drawIndexedInstanced(
        unsigned int indexCount, unsigned int instanceCount, const void* instanceData, std::size_t instanceDataSize
    ) = 0;
    virtual void
    drawIndexedInstanced(unsigned int indexCount, unsigned int instanceCount, IInstanceBuffer& instances) = 0;
    virtual void drawArraysInstanced(unsigned int vertexCount, unsigned int instanceCount)                = 0;
    virtual void setVSync(bool enabled)                                                                   = 0;

    [[nodiscard]] static std::unique_ptr<IRenderDevice> create(GraphicsAPI api);

protected:
    IRenderDevice() = default;
};