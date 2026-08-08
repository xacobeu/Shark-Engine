#pragma once

#include "../ITexture.hpp"
#include "Object.hpp"
#include <string>

namespace GL {

class Texture : public ITexture {
public:
    Texture() = default;
    explicit Texture(const std::string& filepath);
    ~Texture() override = default;

    // Disable copy
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&&) noexcept = default;
    Texture& operator=(Texture&&) noexcept = default;

    void bind(unsigned int slot = 0) const override;
    void unbind() const override;

    [[nodiscard]] int getWidth() const override { return width; }
    [[nodiscard]] int getHeight() const override { return height; }
    [[nodiscard]] GLuint getId() const { return glTexture.get(); }

private:
    GL::TextureObject glTexture;
    int width = 0;
    int height = 0;
    int channels = 0;
};

} // namespace GL
