module;

#ifdef SHARK_NO_GLAD
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

module shark:renderer.backends.opengl.UniformRing;

import std;

import :renderer.backends.opengl.Object;

namespace shark::GL {

// Per-frame linear allocator for uniform data: every push gets its own aligned range, so draws never wait on
// a block being overwritten. Mirrors dynamic uniform offsets in Vulkan and WebGPU.
class UniformRing {
public:
    void reset();
    void push(GLuint binding, const void* data, std::size_t size);

private:
    static constexpr std::size_t MIN_CAPACITY = 64 * 1024;

    GL::UniformBufferObject buffer;
    // Buffers outgrown mid-frame stay alive until the next frame, since bindings made earlier still point at them.
    std::vector<GL::UniformBufferObject> retired;
    std::size_t                          capacity  = 0;
    std::size_t                          offset    = 0;
    std::size_t                          alignment = 0;
};

} // namespace shark::GL

namespace shark::GL {

void UniformRing::reset() {
    retired.clear();
    offset = 0;
    if (capacity != 0) {
        // Orphan last frame's storage instead of waiting for the GPU to finish reading it.
        buffer.bind();
        buffer.data(static_cast<GLsizeiptr>(capacity), nullptr, GL_STREAM_DRAW);
    }
}

void UniformRing::push(GLuint binding, const void* data, std::size_t size) {
    if (alignment == 0) {
        GLint queried = 0;
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &queried);
        alignment = static_cast<std::size_t>(std::max(queried, 1));
    }

    std::size_t start = (offset + alignment - 1) / alignment * alignment;
    if (buffer.get() == 0 || start + size > capacity) {
        if (buffer.get() != 0) {
            retired.push_back(std::move(buffer));
        }
        capacity = std::max({capacity * 2, size, MIN_CAPACITY});
        buffer.create();
        buffer.bind();
        buffer.data(static_cast<GLsizeiptr>(capacity), nullptr, GL_STREAM_DRAW);
        start = 0;
    }

    buffer.bind();
    glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(start), static_cast<GLsizeiptr>(size), data);
    glBindBufferRange(
        GL_UNIFORM_BUFFER, binding, buffer.get(), static_cast<GLintptr>(start), static_cast<GLsizeiptr>(size)
    );
    offset = start + size;
}

} // namespace shark::GL
