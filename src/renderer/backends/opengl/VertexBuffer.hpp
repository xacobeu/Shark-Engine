#pragma once
#include "../IVertexBuffer.hpp"
#include "Object.hpp"

namespace GL {

class VertexBuffer : public IVertexBuffer {
public:
    VertexBuffer(const void* data, size_t size);
    ~VertexBuffer() override = default;

    VertexBuffer(const VertexBuffer&)            = delete ("would double-delete the GL object; move it");
    VertexBuffer& operator=(const VertexBuffer&) = delete ("would double-delete the GL object; move it");

    VertexBuffer(VertexBuffer&&) noexcept            = default;
    VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

    void bind() override;
    void unbind() override;
    void setData(const void* data, size_t size) override;

    [[nodiscard]] GLuint getVaoId() const { return vao.get(); }
    [[nodiscard]] GLuint getVboId() const { return vbo.get(); }

private:
    GL::VertexArrayObject  vao;
    GL::VertexBufferObject vbo;
};

} // namespace GL
