module shark:renderer.backends.opengl.VertexBuffer;

import std;

import :renderer.backends.IVertexBuffer;
import :renderer.backends.opengl.Object;

namespace shark::GL {

class VertexBuffer : public IVertexBuffer {
public:
    VertexBuffer(const void* data, std::size_t size);
    ~VertexBuffer() override = default;

    VertexBuffer(const VertexBuffer&)            = delete ("would double-delete the GL object; move it");
    VertexBuffer& operator=(const VertexBuffer&) = delete ("would double-delete the GL object; move it");

    VertexBuffer(VertexBuffer&&) noexcept            = default;
    VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

    void bind() override;
    void unbind() override;
    void setData(const void* data, std::size_t size) override;

private:
    GL::VertexArrayObject  vao;
    GL::VertexBufferObject vbo;
};

} // namespace shark::GL
