export module shark:renderer.backends.IVertexBuffer;

import std;

export namespace shark {
class IVertexBuffer {
public:
    virtual ~IVertexBuffer() = default;

    IVertexBuffer(const IVertexBuffer&)            = delete ("GPU resources are shared via std::shared_ptr");
    IVertexBuffer& operator=(const IVertexBuffer&) = delete ("GPU resources are shared via std::shared_ptr");

    IVertexBuffer(IVertexBuffer&&)            = default;
    IVertexBuffer& operator=(IVertexBuffer&&) = default;

    virtual void bind()                                      = 0;
    virtual void unbind()                                    = 0;
    virtual void setData(const void* data, std::size_t size) = 0;

protected:
    IVertexBuffer() = default;
};

} // namespace shark
