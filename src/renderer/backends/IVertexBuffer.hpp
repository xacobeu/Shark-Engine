#pragma once
#include <cstddef>

class IVertexBuffer {
public:
    virtual ~IVertexBuffer() = default;

    IVertexBuffer(const IVertexBuffer&)            = delete;
    IVertexBuffer& operator=(const IVertexBuffer&) = delete;

    IVertexBuffer(IVertexBuffer&&)            = default;
    IVertexBuffer& operator=(IVertexBuffer&&) = default;

    virtual void bind()                                 = 0;
    virtual void unbind()                               = 0;
    virtual void setData(const void* data, size_t size) = 0;

protected:
    IVertexBuffer() = default;
};
