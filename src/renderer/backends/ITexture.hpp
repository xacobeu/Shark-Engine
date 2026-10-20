#pragma once
#include <string>

class ITexture {
public:
    virtual ~ITexture()                                         = default;
    virtual void              bind(unsigned int slot = 0) const = 0;
    virtual void              unbind() const                    = 0;
    [[nodiscard]] virtual int getWidth() const                  = 0;
    [[nodiscard]] virtual int getHeight() const                 = 0;
};
