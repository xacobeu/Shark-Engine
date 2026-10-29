module;

#include "GLHeader.hpp"

module shark:renderer.backends.opengl.Shader;

import std;

import :renderer.backends.IShader;

namespace shark::GL {

class Shader : public IShader {
public:
    struct SourceStringTag {};
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    Shader(SourceStringTag, const std::string& vertexSource, const std::string& fragmentSource);
    ~Shader() override;

    // Disable copy
    Shader(const Shader&)            = delete ("would double-delete the GL object; move it");
    Shader& operator=(const Shader&) = delete ("would double-delete the GL object; move it");

    Shader(Shader&&) noexcept            = default;
    Shader& operator=(Shader&&) noexcept = default;

    void bind() override;
    void unbind() override;

    void setMat4(const std::string& name, const float* matrixData) override;
    void setInt(const std::string& name, int value) override;
    void setFloat(const std::string& name, float value) override;
    void setVec2(const std::string& name, float x, float y) override;
    void setVec3(const std::string& name, float x, float y, float z) override;
    void setVec4(const std::string& name, float x, float y, float z, float w) override;
    void setVec4Array(const std::string& name, const float* values, std::size_t count) override;
    void setUniformBlock(const std::string& name, const void* data, std::size_t size) override;

    [[nodiscard]] inline GLuint id() const { return program; }

private:
    struct UniformBlock {
        GLuint      buffer   = 0;
        GLuint      binding  = 0;
        std::size_t capacity = 0;
    };

    GLint              uniform(const std::string& name);
    UniformBlock*      uniformBlock(const std::string& name);
    static std::string loadTextFile(const std::string& path);
    static GLuint      compile(GLenum type, const std::string& source);

    GLuint                                        program = 0;
    std::unordered_map<std::string, GLint>        uniformCache;
    std::unordered_map<std::string, UniformBlock> uniformBlocks;
};

} // namespace shark::GL
