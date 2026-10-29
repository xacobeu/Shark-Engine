export module shark:renderer.backends.IInstanceBuffer;

import std;

export namespace shark {
class IInstanceBuffer {
public:
    virtual ~IInstanceBuffer() = default;

    IInstanceBuffer(const IInstanceBuffer&)            = delete ("GPU resources are shared via std::shared_ptr");
    IInstanceBuffer& operator=(const IInstanceBuffer&) = delete ("GPU resources are shared via std::shared_ptr");

    IInstanceBuffer(IInstanceBuffer&&)            = default;
    IInstanceBuffer& operator=(IInstanceBuffer&&) = default;

    virtual void setData(const void* data, std::size_t size) = 0;

protected:
    IInstanceBuffer() = default;
};

} // namespace shark
