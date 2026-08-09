#pragma once

#include "../IRenderDevice.hpp"

namespace GL {

class RenderDevice : public IRenderDevice {
  public:
    RenderDevice() = default;
    ~RenderDevice() override = default;

    std::shared_ptr<IVertexBuffer> createVertexBuffer(const void *data, size_t size) override;
    std::shared_ptr<IIndexBuffer> createIndexBuffer(const void *data, size_t count) override;
    std::shared_ptr<IShader> createShader(const std::string &vertexPath, const std::string &fragmentPath) override;
    std::shared_ptr<IShader> createShaderFromSource(const std::string &vertexSource, const std::string &fragmentSource) override;
    std::shared_ptr<ITexture> createTexture(const std::string &filepath) override;

    void begin(float r, float g, float b, float a) override;
    void setViewport(int x, int y, int width, int height) override;
    void drawIndexed(unsigned int indexCount) override;
    void drawArrays(unsigned int vertexCount) override;
    void setVSync(bool enabled) override;
};

} // namespace GL
