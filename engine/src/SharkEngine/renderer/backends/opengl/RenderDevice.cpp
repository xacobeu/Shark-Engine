#include <SharkEngine/renderer/backends/opengl/RenderDevice.hpp>
#include <SharkEngine/renderer/backends/opengl/VertexBuffer.hpp>
#include <SharkEngine/renderer/backends/opengl/IndexBuffer.hpp>
#include <SharkEngine/renderer/backends/opengl/Shader.hpp>
#include <SharkEngine/renderer/backends/opengl/Texture.hpp>
#ifdef SHARK_NO_GLAD
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

namespace GL {

std::shared_ptr<IVertexBuffer> RenderDevice::createVertexBuffer(const void* data, size_t size) {
    return std::make_shared<GL::VertexBuffer>(data, size);
}

std::shared_ptr<IIndexBuffer> RenderDevice::createIndexBuffer(const void* data, size_t count) {
    return std::make_shared<GL::IndexBuffer>(static_cast<const unsigned int*>(data), count);
}

std::shared_ptr<IShader> RenderDevice::createShader(const std::string& vertexPath, const std::string& fragmentPath) {
    return std::make_shared<GL::Shader>(vertexPath, fragmentPath);
}

std::shared_ptr<IShader> RenderDevice::createShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    return std::make_shared<GL::Shader>(GL::Shader::SourceStringTag{}, vertexSource, fragmentSource);
}

std::shared_ptr<ITexture> RenderDevice::createTexture(const std::string& filepath) {
    return std::make_shared<GL::Texture>(filepath);
}

void RenderDevice::begin(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

void RenderDevice::setViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void RenderDevice::drawIndexed(unsigned int indexCount) {
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
}

void RenderDevice::drawArrays(unsigned int vertexCount) {
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
}

void RenderDevice::setVSync(bool enabled) {
#ifndef __EMSCRIPTEN__
    glfwSwapInterval(enabled ? 1 : 0);
#endif
}

} // namespace GL
