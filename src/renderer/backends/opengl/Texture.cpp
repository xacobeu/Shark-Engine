#include "renderer/backends/opengl/Texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <print>
#include <stb_image.h>
#include <stdexcept>

namespace GL {

Texture::Texture(const std::string& filepath) {
    glTexture.create();

    // OpenGL amazing defaults wow
    stbi_set_flip_vertically_on_load(1);

    int w = 0, h = 0, ch = 0;

    unsigned char* data = stbi_load(filepath.c_str(), &w, &h, &ch, 0);
    if (data == nullptr) {
        data = stbi_load(("build/desktop-dev/example/" + filepath).c_str(), &w, &h, &ch, 0);
    }
    if (data == nullptr) {
        data = stbi_load(("example/" + filepath).c_str(), &w, &h, &ch, 0);
    }
    if (data == nullptr) {
        std::println(stderr, "[ERROR] Failed to load texture at path: {}", filepath);
        throw std::runtime_error("Failed to load texture: " + filepath);
    }

    width    = w;
    height   = h;
    channels = ch;

    GLenum internalFormat = 0;
    GLenum dataFormat     = 0;
    if (channels == 4) {
        internalFormat = GL_RGBA8;
        dataFormat     = GL_RGBA;
    } else if (channels == 3) {
        internalFormat = GL_RGB8;
        dataFormat     = GL_RGB;
    } else if (channels == 1) {
        internalFormat = GL_R8;
        dataFormat     = GL_RED;
    } else {
        stbi_image_free(data);
        throw std::runtime_error("Unsupported texture format with channels: " + std::to_string(channels));
    }

    glTexture.bind(GL_TEXTURE_2D);

    // Configure wrapping/filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload texture data
    glTexImage2D(
        GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glTexture.bind(GL_TEXTURE_2D);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace GL
