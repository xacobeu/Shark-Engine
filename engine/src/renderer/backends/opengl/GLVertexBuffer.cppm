module;
#ifdef SHARK_NO_GLAD
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif
#include <cstddef>

export module SharkEngine:GLVertexBuffer;
import :RenderDeviceAbstractions;
import :GLObject;
import :Vertex;

export namespace GL {

class VertexBuffer : public IVertexBuffer {
public:
    VertexBuffer(const void* data, size_t size) {
        vao.create();
        vbo.create();

        vao.bind();
        vbo.bind();
        vbo.data(static_cast<GLsizeiptr>(size), data, GL_STATIC_DRAW);

        // Set layout for Vertex directly
        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));

        glEnableVertexAttribArray(1); // normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

        glEnableVertexAttribArray(2); // uv
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, uv)));

        glEnableVertexAttribArray(3); // color
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));

        vao.unbind();
        vbo.unbind();
    }

    ~VertexBuffer() override = default;

    // Disable copy
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&&) noexcept = default;
    VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

    void bind() override {
        vao.bind();
    }

    void unbind() override {
        vao.unbind();
    }

    void setData(const void* data, size_t size) override {
        vbo.bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
    }

    [[nodiscard]] GLuint getVaoId() const { return vao.get(); }
    [[nodiscard]] GLuint getVboId() const { return vbo.get(); }

private:
    GL::VertexArrayObject vao;
    GL::VertexBufferObject vbo;
};

} // namespace GL
