#include "renderer/backends/opengl/InstanceBuffer.hpp"

#include <algorithm>

namespace GL {

void InstanceBuffer::setData(const void* data, std::size_t size) {
    vbo.create();
    vbo.bind();
    if (size > capacity) {
        capacity = std::max(size, capacity * 2);
        vbo.data(static_cast<GLsizeiptr>(capacity), nullptr, GL_DYNAMIC_DRAW);
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
    vbo.unbind();
}

} // namespace GL
