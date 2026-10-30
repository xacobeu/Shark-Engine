module;

#ifdef SHARK_NO_GLAD
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

#include <stb_image.h>

module shark:renderer.backends.opengl.Texture;

import std;

import :renderer.backends.ITexture;
import :renderer.backends.opengl.Object;
import :util.ResourcePath;

namespace shark::GL {

class Texture : public ITexture {
public:
    Texture() = default;
    explicit Texture(const std::string& filepath);
    ~Texture() override = default;

    Texture(const Texture&)            = delete ("would double-delete the GL object; move it");
    Texture& operator=(const Texture&) = delete ("would double-delete the GL object; move it");

    Texture(Texture&&) noexcept            = default;
    Texture& operator=(Texture&&) noexcept = default;

    [[nodiscard]] inline int    getWidth() const override { return width; }
    [[nodiscard]] inline int    getHeight() const override { return height; }
    [[nodiscard]] inline GLuint id() const { return glTexture.get(); }

private:
    GL::TextureObject glTexture;
    int               width    = 0;
    int               height   = 0;
    int               channels = 0;
};

} // namespace shark::GL

namespace shark::GL {

namespace {

struct StbImage {
    std::unique_ptr<unsigned char, decltype(&stbi_image_free)> pixels{nullptr, &stbi_image_free};
    int                                                        width    = 0;
    int                                                        height   = 0;
    int                                                        channels = 0;

    explicit operator bool() const noexcept { return pixels != nullptr; }
};

[[nodiscard]] StbImage loadImage(const std::string& path) {
    // OpenGL amazing defaults wow
    stbi_set_flip_vertically_on_load(1);

    StbImage image;
    image.pixels.reset(stbi_load(path.c_str(), &image.width, &image.height, &image.channels, 0));
    return image;
}

} // namespace

Texture::Texture(const std::string& filepath) {
    if (auto [pixels, w, h, ch] = loadImage(resolveResourcePath(filepath))) {
        GLenum internalFormat = 0;
        GLenum dataFormat     = 0;
        if (ch == 4) {
            internalFormat = GL_RGBA8;
            dataFormat     = GL_RGBA;
        } else if (ch == 3) {
            internalFormat = GL_RGB8;
            dataFormat     = GL_RGB;
        } else if (ch == 1) {
            internalFormat = GL_R8;
            dataFormat     = GL_RED;
        } else {
            throw std::runtime_error("Unsupported texture format with channels: " + std::to_string(ch));
        }

        width    = w;
        height   = h;
        channels = ch;

        glTexture.create();
        glTexture.bind(GL_TEXTURE_2D);

        // Configure wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload texture data
        glTexImage2D(
            GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), width, height, 0, dataFormat, GL_UNSIGNED_BYTE,
            pixels.get()
        );
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        std::println(std::cerr, "[ERROR] Failed to load texture at path: {}", filepath);
        throw std::runtime_error("Failed to load texture: " + filepath);
    }
}

} // namespace shark::GL
