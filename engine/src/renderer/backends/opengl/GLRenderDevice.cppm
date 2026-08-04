module;
#ifdef SHARK_NO_GLAD
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#include <memory>
#include <stdexcept>
#include <string>

export module SharkEngine:GLRenderDevice;
import :RenderDeviceAbstractions;
import :GLVertexBuffer;
import :GLIndexBuffer;
import :GLShader;
import :GLTexture;

export namespace GL {

class RenderDevice : public IRenderDevice {
public:
    RenderDevice() = default;
    ~RenderDevice() override = default;

    std::shared_ptr<IVertexBuffer> createVertexBuffer(const void* data, size_t size) override {
        return std::make_shared<GL::VertexBuffer>(data, size);
    }

    std::shared_ptr<IIndexBuffer> createIndexBuffer(const void* data, size_t count) override {
        return std::make_shared<GL::IndexBuffer>(static_cast<const unsigned int*>(data), count);
    }

    std::shared_ptr<IShader> createShader(const std::string& vertexPath, const std::string& fragmentPath) override {
        return std::make_shared<GL::Shader>(vertexPath, fragmentPath);
    }

    std::shared_ptr<IShader> createShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) override {
        return std::make_shared<GL::Shader>(GL::Shader::SourceStringTag{}, vertexSource, fragmentSource);
    }

    std::shared_ptr<ITexture> createTexture(const std::string& filepath) override {
        return std::make_shared<GL::Texture>(filepath);
    }

    void begin(float r, float g, float b, float a) override {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
    }

    void endFrame() override {}

    void setViewport(int x, int y, int width, int height) override {
        glViewport(x, y, width, height);
    }

    void drawIndexed(unsigned int indexCount) override {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
    }

    void drawArrays(unsigned int vertexCount) override {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
    }

    void setVSync(bool enabled) override {
#ifndef __EMSCRIPTEN__
        glfwSwapInterval(enabled ? 1 : 0);
#else
        (void)enabled;
#endif
    }
};

} // namespace GL

// Out-of-line factory function implementation
std::unique_ptr<IRenderDevice> IRenderDevice::create(GraphicsAPI api) {
    switch (api) {
        case GraphicsAPI::OpenGL:
            return std::make_unique<GL::RenderDevice>();
        default:
            throw std::runtime_error("Unsupported Graphics API!");
    }
}
