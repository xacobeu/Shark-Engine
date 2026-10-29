module shark:renderer.backends.opengl.Texture;

import std;

import :renderer.backends.ITexture;
import :renderer.backends.opengl.Object;

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

    void bind(unsigned int slot = 0) const override;
    void unbind() const override;

    [[nodiscard]] inline int getWidth() const override { return width; }
    [[nodiscard]] inline int getHeight() const override { return height; }

private:
    GL::TextureObject glTexture;
    int               width    = 0;
    int               height   = 0;
    int               channels = 0;
};

} // namespace shark::GL
