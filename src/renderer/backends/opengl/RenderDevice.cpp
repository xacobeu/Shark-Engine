#include "renderer/backends/opengl/RenderDevice.hpp"

#include <bit>
#include <cstddef>
#include <cstdint>

#include "renderer/backends/opengl/IndexBuffer.hpp"
#include "renderer/backends/opengl/Shader.hpp"
#include "renderer/backends/opengl/Texture.hpp"
#include "renderer/backends/opengl/VertexBuffer.hpp"

#ifdef SHARK_NO_GLAD
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

namespace GL {

std::shared_ptr<IVertexBuffer> RenderDevice::createVertexBuffer(const void* data, std::size_t size) {
    return std::make_shared<GL::VertexBuffer>(data, size);
}

std::shared_ptr<IIndexBuffer> RenderDevice::createIndexBuffer(const void* data, std::size_t count) {
    return std::make_shared<GL::IndexBuffer>(static_cast<const unsigned int*>(data), count);
}

std::shared_ptr<IShader> RenderDevice::createShader(const std::string& vertexPath, const std::string& fragmentPath) {
    return std::make_shared<GL::Shader>(vertexPath, fragmentPath);
}

std::shared_ptr<IShader>
RenderDevice::createShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    return std::make_shared<GL::Shader>(GL::Shader::SourceStringTag{}, vertexSource, fragmentSource);
}

std::shared_ptr<ITexture> RenderDevice::createTexture(const std::string& filepath) {
    return std::make_shared<GL::Texture>(filepath);
}

std::shared_ptr<IInstanceBuffer> RenderDevice::createInstanceBuffer() {
    return std::make_shared<GL::InstanceBuffer>();
}

void RenderDevice::begin(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

void RenderDevice::setViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

namespace {
[[nodiscard]] auto bufferOffset(std::size_t offset) noexcept -> const void* {
    return std::bit_cast<const void*>(static_cast<std::uintptr_t>(offset));
}
} // namespace

void RenderDevice::drawIndexed(unsigned int indexCount) {
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
}

void RenderDevice::drawArrays(unsigned int vertexCount) {
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
}

void RenderDevice::drawIndexedInstanced(
    unsigned int indexCount, unsigned int instanceCount, const void* instanceData, std::size_t instanceDataSize
) {
    if (instanceCount == 0 || (instanceData == nullptr) || instanceDataSize == 0) {
        return;
    }

    scratchInstances.setData(instanceData, instanceDataSize);
    drawIndexedInstanced(indexCount, instanceCount, scratchInstances);
}

void RenderDevice::drawIndexedInstanced(
    unsigned int indexCount, unsigned int instanceCount, IInstanceBuffer& instances
) {
    if (instanceCount == 0) {
        return;
    }

    const auto& buffer = dynamic_cast<const GL::InstanceBuffer&>(instances);
    buffer.bind();

    constexpr auto stride = static_cast<GLsizei>(sizeof(float) * 20); // 4x4 matrix (16 floats) + vec4 color (4 floats)

    // Locations 4, 5, 6, 7: mat4 modelMatrix (4 vec4s)
    for (unsigned int i = 0; i < 4; ++i) {
        GLuint loc = 4 + i;
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, stride, bufferOffset(sizeof(float) * 4 * i));
        glVertexAttribDivisor(loc, 1);
    }

    // Location 8: vec4 color
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, stride, bufferOffset(sizeof(float) * 16));
    glVertexAttribDivisor(8, 1);

    glDrawElementsInstanced(
        GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(instanceCount)
    );

    // Clean up divisors and disable instance attributes so standard draws are unaffected
    for (unsigned int i = 0; i < 4; ++i) {
        glVertexAttribDivisor(4 + i, 0);
        glDisableVertexAttribArray(4 + i);
    }
    glVertexAttribDivisor(8, 0);
    glDisableVertexAttribArray(8);

    buffer.unbind();
}

void RenderDevice::drawArraysInstanced(unsigned int vertexCount, unsigned int instanceCount) {
    glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount), static_cast<GLsizei>(instanceCount));
}

void RenderDevice::setVSync(bool enabled) {
#ifndef __EMSCRIPTEN__
    glfwSwapInterval(enabled ? 1 : 0);
#else
    (void)enabled;
#endif
}

} // namespace GL