#include "renderer/backends/opengl/VertexBuffer.hpp"

#include <bit>
#include <cstddef>
#include <cstdint>

#include "renderer/geometry/Vertex.hpp"
#ifdef SHARK_NO_GLAD
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

namespace GL {

namespace {

[[nodiscard]] auto bufferOffset(std::size_t offset) noexcept -> const void* {
    return std::bit_cast<const void*>(static_cast<std::uintptr_t>(offset));
}

} // namespace

VertexBuffer::VertexBuffer(const void* data, size_t size) {
    vao.create();
    vbo.create();

    vao.bind();
    vbo.bind();
    vbo.data(static_cast<GLsizeiptr>(size), data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), bufferOffset(offsetof(Vertex, position)));

    glEnableVertexAttribArray(1); // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), bufferOffset(offsetof(Vertex, normal)));

    glEnableVertexAttribArray(2); // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), bufferOffset(offsetof(Vertex, uv)));

    glEnableVertexAttribArray(3); // color
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), bufferOffset(offsetof(Vertex, color)));

    vao.unbind();
    vbo.unbind();
}

void VertexBuffer::bind() {
    vao.bind();
}

void VertexBuffer::unbind() {
    vao.unbind();
}

void VertexBuffer::setData(const void* data, size_t size) {
    vbo.bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
}

} // namespace GL
