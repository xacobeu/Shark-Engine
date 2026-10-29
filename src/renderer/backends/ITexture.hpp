#pragma once

class ITexture {
public:
    virtual ~ITexture() = default;

    ITexture(const ITexture&)            = delete;
    ITexture& operator=(const ITexture&) = delete;

    ITexture(ITexture&&)            = default;
    ITexture& operator=(ITexture&&) = default;

    virtual void              bind(unsigned int slot = 0) const = 0;
    virtual void              unbind() const                    = 0;
    [[nodiscard]] virtual int getWidth() const                  = 0;
    [[nodiscard]] virtual int getHeight() const                 = 0;

protected:
    ITexture() = default;
};
