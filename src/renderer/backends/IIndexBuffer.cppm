export module shark:renderer.backends.IIndexBuffer;

import std;

export namespace shark {
class IIndexBuffer {
public:
    virtual ~IIndexBuffer() = default;

    IIndexBuffer(const IIndexBuffer&)            = delete ("GPU resources are shared via std::shared_ptr");
    IIndexBuffer& operator=(const IIndexBuffer&) = delete ("GPU resources are shared via std::shared_ptr");

    IIndexBuffer(IIndexBuffer&&)            = default;
    IIndexBuffer& operator=(IIndexBuffer&&) = default;

    virtual void                      bind()                                               = 0;
    virtual void                      unbind()                                             = 0;
    virtual void                      setData(const unsigned int* data, std::size_t count) = 0;
    [[nodiscard]] virtual std::size_t getIndexCount() const                                = 0;

protected:
    IIndexBuffer() = default;
};

} // namespace shark
