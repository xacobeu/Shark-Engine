#pragma once

#include "../IShader.hpp"
#ifdef SHARK_NO_GLAD
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif
#include <string>
#include <unordered_map>

namespace GL {

class Shader : public IShader {
public:
    struct SourceStringTag {};
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    Shader(SourceStringTag, const std::string& vertexSource, const std::string& fragmentSource);
    ~Shader() override;

    // Disable copy
    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;

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

    [[nodiscard]] GLuint id() const { return program; }

private:
    GLint              uniform(const std::string& name);
    static std::string loadTextFile(const std::string& path);
    static GLuint      compile(GLenum type, const std::string& source);

    GLuint                                 program = 0;
    std::unordered_map<std::string, GLint> uniformCache;
};

} // namespace GL
