#pragma once
#include "../IIndexBuffer.hpp"
#include "Object.hpp"

namespace GL {

class IndexBuffer : public IIndexBuffer {
public:
    IndexBuffer(const unsigned int* data, size_t count);
    ~IndexBuffer() override = default;

    IndexBuffer(const IndexBuffer&)            = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    IndexBuffer(IndexBuffer&&) noexcept            = default;
    IndexBuffer& operator=(IndexBuffer&&) noexcept = default;

    void bind() override;
    void unbind() override;
    void setData(const unsigned int* data, size_t count) override;

    [[nodiscard]] size_t getIndexCount() const override { return indexCount; }
    [[nodiscard]] GLuint getEboId() const { return ebo.get(); }

private:
    GL::ElementBufferObject ebo;
    size_t                  indexCount = 0;
};

} // namespace GL
