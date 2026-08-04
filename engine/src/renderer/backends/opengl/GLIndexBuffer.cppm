module;
#ifdef SHARK_NO_GLAD
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif
#include <cstddef>

export module SharkEngine:GLIndexBuffer;
import :RenderDeviceAbstractions;
import :GLObject;

export namespace GL {

class IndexBuffer : public IIndexBuffer {
public:
    IndexBuffer(const unsigned int* data, size_t count) : indexCount(count) {
        ebo.create();
        ebo.bind();
        ebo.data(static_cast<GLsizeiptr>(count * sizeof(unsigned int)), data, GL_STATIC_DRAW);
    }

    ~IndexBuffer() override = default;

    // Disable copy
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    IndexBuffer(IndexBuffer&&) noexcept = default;
    IndexBuffer& operator=(IndexBuffer&&) noexcept = default;

    void bind() override {
        ebo.bind();
    }

    void unbind() override {
        // Note: unbinding EBO while VAO is bound will unbind it from VAO.
        // Generally, let VAO unbind handle it, or unbind raw target.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void setData(const unsigned int* data, size_t count) override {
        ebo.bind();
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(count * sizeof(unsigned int)), data);
        indexCount = count;
    }

    [[nodiscard]] size_t getIndexCount() const override { return indexCount; }
    [[nodiscard]] GLuint getEboId() const { return ebo.get(); }

private:
    GL::ElementBufferObject ebo;
    size_t indexCount = 0;
};

} // namespace GL
