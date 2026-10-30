module;

#ifdef SHARK_NO_GLAD
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

module shark:renderer.backends.opengl.InstanceBuffer;

import std;

import :renderer.backends.IInstanceBuffer;
import :renderer.backends.opengl.Object;

namespace shark::GL {

// Storage only grows, so once it's big enough an update is a single glBufferSubData with no reallocation.
class InstanceBuffer final : public IInstanceBuffer {
public:
    void setData(const void* data, std::size_t size) override;

    inline void bind() const { vbo.bind(); }
    inline void unbind() const { vbo.unbind(); }

private:
    GL::VertexBufferObject vbo;
    std::size_t            capacity = 0;
};

} // namespace shark::GL

namespace shark::GL {

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

} // namespace shark::GL
