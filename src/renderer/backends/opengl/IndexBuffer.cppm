module shark:renderer.backends.opengl.IndexBuffer;

import std;

import :renderer.backends.IIndexBuffer;
import :renderer.backends.opengl.Object;

namespace shark::GL {

class IndexBuffer : public IIndexBuffer {
public:
    IndexBuffer(const unsigned int* data, std::size_t count);
    ~IndexBuffer() override = default;

    IndexBuffer(const IndexBuffer&)            = delete ("would double-delete the GL object; move it");
    IndexBuffer& operator=(const IndexBuffer&) = delete ("would double-delete the GL object; move it");

    IndexBuffer(IndexBuffer&&) noexcept            = default;
    IndexBuffer& operator=(IndexBuffer&&) noexcept = default;

    void bind() override;
    void unbind() override;
    void setData(const unsigned int* data, std::size_t count) override;

    [[nodiscard]] inline std::size_t getIndexCount() const override { return indexCount; }

private:
    GL::ElementBufferObject ebo;
    std::size_t             indexCount = 0;
};

} // namespace shark::GL
