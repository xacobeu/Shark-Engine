#pragma once
#include <cstddef>
#include <memory>
#include <string>

#include "renderer/backends/IRenderDevice.hpp"
#include "renderer/backends/opengl/Object.hpp"

namespace GL {

class RenderDevice final : public IRenderDevice {
public:
    RenderDevice()           = default;
    ~RenderDevice() override = default;

    std::shared_ptr<IVertexBuffer> createVertexBuffer(const void* data, std::size_t size) override;
    std::shared_ptr<IIndexBuffer>  createIndexBuffer(const void* data, std::size_t count) override;
    std::shared_ptr<IShader> createShader(const std::string& vertexPath, const std::string& fragmentPath) override;
    std::shared_ptr<IShader>
    createShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) override;
    std::shared_ptr<ITexture> createTexture(const std::string& filepath) override;

    void begin(float r, float g, float b, float a) override;
    void setViewport(int x, int y, int width, int height) override;
    void drawIndexed(unsigned int indexCount) override;
    void drawArrays(unsigned int vertexCount) override;
    void drawIndexedInstanced(
        unsigned int indexCount, unsigned int instanceCount, const void* instanceData, std::size_t instanceDataSize
    ) override;
    void drawArraysInstanced(unsigned int vertexCount, unsigned int instanceCount) override;
    void setVSync(bool enabled) override;

private:
    GL::VertexBufferObject instanceVbo;
};

} // namespace GL