module;

#ifdef SHARK_NO_GLAD
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

module shark:renderer.backends.opengl.Buffer;

import std;

import :renderer.backends.IBuffer;
import :renderer.backends.opengl.Object;

namespace shark::GL {

// Storage only grows, so once it's big enough an update is a single glBufferSubData with no reallocation.
class Buffer final : public IBuffer {
public:
    Buffer(BufferUsage usage, const void* data, std::size_t size);

    void                             setData(const void* data, std::size_t size) override;
    [[nodiscard]] inline std::size_t getSize() const override { return size; }
    [[nodiscard]] inline BufferUsage getUsage() const override { return usage; }

    [[nodiscard]] inline GLenum target() const {
        return usage == BufferUsage::Index ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
    }
    [[nodiscard]] inline GLuint id() const { return buffer.get(); }
    // Unique for the program's lifetime, unlike addresses and GL names which get reused.
    [[nodiscard]] inline std::uint64_t serial() const { return serialNumber; }

private:
    inline static std::uint64_t nextSerial = 0;

    GL::BufferObject buffer;
    std::uint64_t    serialNumber = ++nextSerial;
    BufferUsage      usage;
    std::size_t      size     = 0;
    std::size_t      capacity = 0;
};

} // namespace shark::GL

namespace shark::GL {

Buffer::Buffer(BufferUsage usage, const void* data, std::size_t size) : usage(usage), size(size), capacity(size) {
    buffer.create();
    glBindBuffer(target(), buffer.get());
    glBufferData(target(), static_cast<GLsizeiptr>(size), data, data != nullptr ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
}

void Buffer::setData(const void* data, std::size_t newSize) {
    glBindBuffer(target(), buffer.get());
    if (newSize > capacity) {
        capacity = std::max(newSize, capacity * 2);
        glBufferData(target(), static_cast<GLsizeiptr>(capacity), nullptr, GL_DYNAMIC_DRAW);
    }
    glBufferSubData(target(), 0, static_cast<GLsizeiptr>(newSize), data);
    size = newSize;
}

} // namespace shark::GL
