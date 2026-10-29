module;

#include "GLHeader.hpp"

module shark:renderer.backends.opengl.Object;

import std;

namespace shark::GL {

[[nodiscard]] inline auto bufferOffset(std::size_t offset) noexcept -> const void* {
    return std::bit_cast<const void*>(static_cast<std::uintptr_t>(offset));
}

template <typename T> class Object {
public:
    Object() = default;

    void create() {
        if (id == 0)
            T::Gen(1, &id);
    }

    template <typename... Args>
        requires requires(GLuint id, Args&&... as) { T::Bind(id, std::forward<Args>(as)...); }
    void bind(Args&&... args) const {
        T::Bind(id, std::forward<Args>(args)...);
    }

    template <typename... Args>
        requires requires(GLsizeiptr s, const void* p, GLenum u) { T::Data(s, p, u); }
    void data(GLsizeiptr size, const void* pointer, GLenum usage) const {
        T::Data(size, pointer, usage);
    }

    void unbind() const
        requires requires { T::Unbind(); }
    {
        T::Unbind();
    }

    ~Object() {
        if (id != 0)
            T::Del(1, &id);
    }

    Object(Object&& other) noexcept : id(other.id) { other.id = 0; }

    Object& operator=(Object&& other) noexcept {
        if (this != &other) {
            if (id != 0)
                T::Del(1, &id);
            id       = other.id;
            other.id = 0;
        }
        return *this;
    }

    GLuint get() const { return id; }

private:
    GLuint id = 0;

    Object(const Object&)            = delete ("would double-delete the GL object; move it");
    Object& operator=(const Object&) = delete ("would double-delete the GL object; move it");
};

struct VertexArrayOperations {
    static inline void Gen(GLsizei n, GLuint* ids) { glGenVertexArrays(n, ids); }
    static inline void Del(GLsizei n, const GLuint* ids) { glDeleteVertexArrays(n, ids); }
    static inline void Bind(GLuint id) { glBindVertexArray(id); }
    static inline void Unbind() { glBindVertexArray(0); }

    static inline void
    Attrib(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, std::size_t offset) {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, type, normalized, stride, bufferOffset(offset));
    }
};

template <GLenum target> struct BufferOperations {
    static void Gen(GLsizei n, GLuint* ids) { glGenBuffers(n, ids); }
    static void Del(GLsizei n, const GLuint* ids) { glDeleteBuffers(n, ids); }
    static void Bind(GLuint id) { glBindBuffer(target, id); }
    static void Unbind() { glBindBuffer(target, 0); }

    static void Data(GLsizeiptr size, const void* data, GLenum usage) { glBufferData(target, size, data, usage); }
};

struct TextureOperations {
    static inline void Gen(GLsizei n, GLuint* ids) { glGenTextures(n, ids); }
    static inline void Del(GLsizei n, const GLuint* ids) { glDeleteTextures(n, ids); }
    static inline void Bind(GLuint id, GLenum target = GL_TEXTURE_2D) { glBindTexture(target, id); }
};

using TextureObject       = Object<TextureOperations>;
using VertexBufferObject  = Object<BufferOperations<GL_ARRAY_BUFFER>>;
using ElementBufferObject = Object<BufferOperations<GL_ELEMENT_ARRAY_BUFFER>>;
using VertexArrayObject   = Object<VertexArrayOperations>;
} // namespace shark::GL
