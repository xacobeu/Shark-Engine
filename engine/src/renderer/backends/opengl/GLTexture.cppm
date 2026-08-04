module;
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#ifdef SHARK_NO_GLAD
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif
#include <string>
#include <iostream>
#include <stdexcept>

export module SharkEngine:GLTexture;
import :RenderDeviceAbstractions;
import :GLObject;

export namespace GL {

class Texture : public ITexture {
public:
    Texture() = default;

    explicit Texture(const std::string& filepath) {
        glTexture.create();

        // Flip textures vertically as OpenGL expects UV coordinates to start from bottom-left
        stbi_set_flip_vertically_on_load(true);

        int w, h, ch;
        unsigned char* data = stbi_load(filepath.c_str(), &w, &h, &ch, 0);
        if (!data) {
            std::cerr << "[ERROR] Failed to load texture at path: " << filepath << std::endl;
            throw std::runtime_error("Failed to load texture: " + filepath);
        }

        width = w;
        height = h;
        channels = ch;

        GLenum internalFormat = 0;
        GLenum dataFormat = 0;
        if (channels == 4) {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        } else if (channels == 3) {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        } else if (channels == 1) {
            internalFormat = GL_R8;
            dataFormat = GL_RED;
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
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    ~Texture() override = default;

    // Disable copy
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&&) noexcept = default;
    Texture& operator=(Texture&&) noexcept = default;

    void bind(unsigned int slot = 0) const override {
        glActiveTexture(GL_TEXTURE0 + slot);
        glTexture.bind(GL_TEXTURE_2D);
    }

    void unbind() const override {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

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
