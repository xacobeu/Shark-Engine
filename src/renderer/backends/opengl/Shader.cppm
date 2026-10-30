module;

#ifdef SHARK_NO_GLAD
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

module shark:renderer.backends.opengl.Shader;

import std;

import :renderer.backends.IShader;
import :util.ResourcePath;

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

namespace shark::GL {

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    : Shader(SourceStringTag{}, loadTextFile(vertexPath), loadTextFile(fragmentPath)) {}

Shader::Shader(SourceStringTag, const std::string& vertexSource, const std::string& fragmentSource)
    : program(glCreateProgram()) {
    GLuint vs = compile(GL_VERTEX_SHADER, vertexSource);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fragmentSource);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == 0) {
        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<std::size_t>(logLength));
        glGetProgramInfoLog(program, logLength, nullptr, log.data());
        std::println(std::cerr, "[ERROR] Link failed: {}", log.data());

        glDeleteShader(vs);
        glDeleteShader(fs);
        glDeleteProgram(program);
        program = 0;
        throw std::runtime_error("Shader program linking failed");
    }

    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader() {
    for (auto& [name, block] : uniformBlocks) {
        if (block.buffer != 0) {
            glDeleteBuffers(1, &block.buffer);
        }
    }
    if (program != 0) {
        glDeleteProgram(program);
    }
}

void Shader::bind() {
    glUseProgram(program);
    for (const auto& [name, block] : uniformBlocks) {
        if (block.buffer != 0) {
            glBindBufferBase(GL_UNIFORM_BUFFER, block.binding, block.buffer);
        }
    }
}

void Shader::unbind() {
    glUseProgram(0);
}

GLint Shader::uniform(const std::string& name) {
    auto it = uniformCache.find(name);
    if (it != uniformCache.end()) {
        return it->second;
    }
    GLint loc          = glGetUniformLocation(program, name.c_str());
    uniformCache[name] = loc;
    return loc;
}

void Shader::setMat4(const std::string& name, const float* matrixData) {
    glUniformMatrix4fv(uniform(name), 1, GL_FALSE, matrixData);
}

void Shader::setInt(const std::string& name, int value) {
    glUniform1i(uniform(name), value);
}

void Shader::setFloat(const std::string& name, float value) {
    glUniform1f(uniform(name), value);
}

void Shader::setVec2(const std::string& name, float x, float y) {
    glUniform2f(uniform(name), x, y);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) {
    glUniform3f(uniform(name), x, y, z);
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) {
    glUniform4f(uniform(name), x, y, z, w);
}

void Shader::setVec4Array(const std::string& name, const float* values, std::size_t count) {
    glUniform4fv(uniform(name), static_cast<GLsizei>(count), values);
}

Shader::UniformBlock* Shader::uniformBlock(const std::string& name) {
    auto it = uniformBlocks.find(name);
    if (it != uniformBlocks.end()) {
        return it->second.buffer != 0 ? &it->second : nullptr;
    }

    UniformBlock& block = uniformBlocks[name];
    const GLuint  index = glGetUniformBlockIndex(program, name.c_str());
    if (index == GL_INVALID_INDEX) {
        return nullptr;
    }

    GLint dataSize = 0;
    glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);

    block.binding  = static_cast<GLuint>(uniformBlocks.size() - 1);
    block.capacity = static_cast<std::size_t>(dataSize);
    glUniformBlockBinding(program, index, block.binding);

    // WebGL rejects draws whose bound buffer is smaller than the block, so allocate it whole.
    glGenBuffers(1, &block.buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, block.buffer);
    glBufferData(GL_UNIFORM_BUFFER, dataSize, nullptr, GL_DYNAMIC_DRAW);
    return &block;
}

void Shader::setUniformBlock(const std::string& name, const void* data, std::size_t size) {
    UniformBlock* block = uniformBlock(name);
    if (block == nullptr) {
        return;
    }
    glBindBuffer(GL_UNIFORM_BUFFER, block->buffer);
    if (size > block->capacity) {
        glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(size), data, GL_DYNAMIC_DRAW);
        block->capacity = size;
    } else {
        glBufferSubData(GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, block->binding, block->buffer);
}

std::string Shader::loadTextFile(const std::string& path) {
    std::ifstream file(resolveResourcePath(path));
    if (!file.is_open()) {
        std::println(std::cerr, "[ERROR] Failed to open file: {}", path);
        throw std::runtime_error("Failed to open shader file: " + path);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GLuint Shader::compile(GLenum type, const std::string& source) {
#ifdef __EMSCRIPTEN__
    std::string processedSource = source;
    std::size_t verPos          = processedSource.find("#version 330 core");
    if (verPos != std::string::npos) {
        processedSource.replace(verPos, 17, "#version 300 es");
    }
    if (processedSource.find("precision ") == std::string::npos) {
        std::size_t versionEnd = processedSource.find('\n');
        if (versionEnd != std::string::npos) {
            processedSource.insert(versionEnd + 1, "precision highp float;\nprecision highp int;\n");
        }
    }
    const char* src = processedSource.c_str();
#else
    const char* src = source.c_str();
#endif
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    GLint compiled = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
    if (compiled == 0) {
        GLint logLength = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<std::size_t>(logLength > 0 ? logLength : 1024));
        glGetShaderInfoLog(id, static_cast<GLsizei>(log.size()), nullptr, log.data());
        std::println(std::cerr, "[ERROR] Shader compilation failed: {}", log.data());
        glDeleteShader(id);
        throw std::runtime_error(std::string("Shader compilation failed: ") + log.data());
    }
    return id;
}

} // namespace shark::GL
