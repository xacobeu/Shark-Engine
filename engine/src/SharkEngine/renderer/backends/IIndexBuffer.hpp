#pragma once
#include <cstddef>

class IIndexBuffer {
public:
    virtual ~IIndexBuffer() = default;
    virtual void bind() = 0;
    virtual void unbind() = 0;
    virtual void setData(const unsigned int* data, size_t count) = 0;
    [[nodiscard]] virtual size_t getIndexCount() const = 0;
};
