module;

#include "GLHeader.hpp"

#include <cstddef>

module shark:renderer.backends.opengl.VertexBuffer.impl;

import std;

import :renderer.geometry.Vertex;

import :renderer.backends.opengl.VertexBuffer;

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
