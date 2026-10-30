export module shark:renderer.backends.IBuffer;

import std;

export namespace shark {
// Index buffers always hold 32-bit unsigned indices. Instance data lives in Vertex buffers stepped per instance.
enum class BufferUsage : std::uint8_t { Vertex, Index };

class IBuffer {
public:
    virtual ~IBuffer() = default;

    IBuffer(const IBuffer&)            = delete ("GPU resources are shared via std::shared_ptr");
    IBuffer& operator=(const IBuffer&) = delete ("GPU resources are shared via std::shared_ptr");

    IBuffer(IBuffer&&)            = default;
    IBuffer& operator=(IBuffer&&) = default;

    // Replaces the contents, growing the storage when needed. Write a buffer at most once per frame:
    // backends that record commands ahead of submission make every draw in the frame see the last write.
    virtual void                      setData(const void* data, std::size_t size) = 0;
    [[nodiscard]] virtual std::size_t getSize() const                             = 0;
    [[nodiscard]] virtual BufferUsage getUsage() const                            = 0;

protected:
    IBuffer() = default;
};

} // namespace shark
