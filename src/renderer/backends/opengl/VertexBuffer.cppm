module;

#ifdef SHARK_NO_GLAD
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

#include <cstddef>

module shark:renderer.backends.opengl.VertexBuffer;

import std;

import :renderer.backends.IVertexBuffer;
import :renderer.backends.opengl.Object;
import :renderer.geometry.Vertex;

namespace shark::GL {

class VertexBuffer : public IVertexBuffer {
public:
    VertexBuffer(const void* data, std::size_t size);
    ~VertexBuffer() override = default;

    VertexBuffer(const VertexBuffer&)            = delete ("would double-delete the GL object; move it");
    VertexBuffer& operator=(const VertexBuffer&) = delete ("would double-delete the GL object; move it");

    VertexBuffer(VertexBuffer&&) noexcept            = default;
    VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

    void bind() override;
    void unbind() override;
    void setData(const void* data, std::size_t size) override;

private:
    GL::VertexArrayObject  vao;
    GL::VertexBufferObject vbo;
};

} // namespace shark::GL

namespace shark::GL {

VertexBuffer::VertexBuffer(const void* data, std::size_t size) {
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

void VertexBuffer::setData(const void* data, std::size_t size) {
    vbo.bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
}

} // namespace shark::GL
