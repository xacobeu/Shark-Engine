#pragma once
#include <cstddef>

class IInstanceBuffer {
public:
    virtual ~IInstanceBuffer() = default;

    IInstanceBuffer(const IInstanceBuffer&)            = delete;
    IInstanceBuffer& operator=(const IInstanceBuffer&) = delete;

    IInstanceBuffer(IInstanceBuffer&&)            = default;
    IInstanceBuffer& operator=(IInstanceBuffer&&) = default;

    virtual void setData(const void* data, std::size_t size) = 0;

protected:
    IInstanceBuffer() = default;
};
