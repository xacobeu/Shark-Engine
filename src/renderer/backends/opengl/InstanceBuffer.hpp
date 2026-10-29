#pragma once
#include <cstddef>

#include "../IInstanceBuffer.hpp"
#include "Object.hpp"

namespace GL {

// Storage only grows, so once it's big enough an update is a single glBufferSubData with no reallocation.
class InstanceBuffer final : public IInstanceBuffer {
public:
    void setData(const void* data, std::size_t size) override;

    void bind() const { vbo.bind(); }
    void unbind() const { vbo.unbind(); }

private:
    GL::VertexBufferObject vbo;
    std::size_t            capacity = 0;
};

} // namespace GL
