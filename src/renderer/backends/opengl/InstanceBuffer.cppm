module shark:renderer.backends.opengl.InstanceBuffer;

import std;

import :renderer.backends.IInstanceBuffer;
import :renderer.backends.opengl.Object;

namespace shark::GL {

// Storage only grows, so once it's big enough an update is a single glBufferSubData with no reallocation.
class InstanceBuffer final : public IInstanceBuffer {
public:
    void setData(const void* data, std::size_t size) override;

    inline void bind() const { vbo.bind(); }
    inline void unbind() const { vbo.unbind(); }

private:
    GL::VertexBufferObject vbo;
    std::size_t            capacity = 0;
};

} // namespace shark::GL
