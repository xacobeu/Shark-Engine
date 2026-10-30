export module shark:renderer.backends.ITexture;

export namespace shark {
class ITexture {
public:
    virtual ~ITexture() = default;

    ITexture(const ITexture&)            = delete ("GPU resources are shared via std::shared_ptr");
    ITexture& operator=(const ITexture&) = delete ("GPU resources are shared via std::shared_ptr");

    ITexture(ITexture&&)            = default;
    ITexture& operator=(ITexture&&) = default;

    [[nodiscard]] virtual int getWidth() const  = 0;
    [[nodiscard]] virtual int getHeight() const = 0;

protected:
    ITexture() = default;
};

} // namespace shark
