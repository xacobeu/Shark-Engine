#pragma once
#include <cstddef>

class IVertexBuffer {
public:
    virtual ~IVertexBuffer() = default;

    IVertexBuffer(const IVertexBuffer&)            = delete ("GPU resources are shared via std::shared_ptr");
    IVertexBuffer& operator=(const IVertexBuffer&) = delete ("GPU resources are shared via std::shared_ptr");

    IVertexBuffer(IVertexBuffer&&)            = default;
    IVertexBuffer& operator=(IVertexBuffer&&) = default;

    virtual void bind()                                 = 0;
    virtual void unbind()                               = 0;
    virtual void setData(const void* data, size_t size) = 0;

protected:
    IVertexBuffer() = default;
};
