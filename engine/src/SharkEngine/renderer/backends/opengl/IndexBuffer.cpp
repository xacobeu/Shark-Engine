#include <SharkEngine/renderer/backends/opengl/IndexBuffer.hpp>

namespace GL {

IndexBuffer::IndexBuffer(const unsigned int *data, size_t count) : indexCount(count) {
    ebo.create();
    ebo.bind();
    ebo.data(static_cast<GLsizeiptr>(count * sizeof(unsigned int)), data, GL_STATIC_DRAW);
}

void IndexBuffer::bind() {
    ebo.bind();
}

void IndexBuffer::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::setData(const unsigned int *data, size_t count) {
    ebo.bind();
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(count * sizeof(unsigned int)), data);
    indexCount = count;
}

} // namespace GL
