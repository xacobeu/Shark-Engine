#pragma once

#include <SharkEngine/renderer/GL/GLObject.hpp>
#include <string>

class Texture {
public:
    Texture() = default;
    explicit Texture(const std::string& filepath);
    ~Texture() = default;

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&&) noexcept = default;
    Texture& operator=(Texture&&) noexcept = default;

    void bind(unsigned int slot = 0) const;
    void unbind() const;

    [[nodiscard]] int getWidth() const { return width; }
    [[nodiscard]] int getHeight() const { return height; }
    [[nodiscard]] GLuint getId() const { return glTexture.get(); }

private:
    GL::TextureObject glTexture;
    int width = 0;
    int height = 0;
    int channels = 0;
};
