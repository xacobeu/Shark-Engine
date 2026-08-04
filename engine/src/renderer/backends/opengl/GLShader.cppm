module;
#ifdef SHARK_NO_GLAD
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <algorithm>

export module SharkEngine:GLShader;
import :RenderDeviceAbstractions;
import :GLObject;

export namespace GL {

class Shader : public IShader {
public:
    struct SourceStringTag {};

    Shader(const std::string& vertexPath, const std::string& fragmentPath) {
        std::string vertexSource = loadTextFile(vertexPath);
        std::string fragmentSource = loadTextFile(fragmentPath);

        GLuint vs = compile(GL_VERTEX_SHADER, vertexSource);
        GLuint fs = compile(GL_FRAGMENT_SHADER, fragmentSource);
        link(vs, fs);
    }

    Shader(SourceStringTag, const std::string& vertexSource, const std::string& fragmentSource) {
        GLuint vs = compile(GL_VERTEX_SHADER, vertexSource);
        GLuint fs = compile(GL_FRAGMENT_SHADER, fragmentSource);
        link(vs, fs);
    }

    ~Shader() override {
        for (auto& block : uboBlocks) {
            if (block.bufferObject) glDeleteBuffers(1, &block.bufferObject);
        }
        if (program != 0) {
            glDeleteProgram(program);
        }
    }

    // Disable copy
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&&) noexcept = default;
    Shader& operator=(Shader&&) noexcept = default;

    void bind() override {
        glUseProgram(program);

        for (size_t i = 0; i < uboBlocks.size(); ++i) {
            UBOBlock& block = uboBlocks[i];
            glBindBuffer(GL_UNIFORM_BUFFER, block.bufferObject);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, block.size, block.shadowBuffer.data());
            glBindBufferBase(GL_UNIFORM_BUFFER, block.bindingPoint, block.bufferObject);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
    }

    void unbind() override {
        glUseProgram(0);
    }

    void setMat4(const std::string& name, const float* matrixData) override {
        if (uboMembers.count(name)) { setUBOField(name, matrixData, 64); return; }
        glUniformMatrix4fv(uniform(name), 1, GL_FALSE, matrixData);
    }

    void setInt(const std::string& name, int value) override {
        if (uboMembers.count(name)) { setUBOField(name, &value, 4); return; }
        glUniform1i(uniform(name), value);
    }

    void setFloat(const std::string& name, float value) override {
        if (uboMembers.count(name)) { setUBOField(name, &value, 4); return; }
        glUniform1f(uniform(name), value);
    }

    void setVec2(const std::string& name, float x, float y) override {
        float v[2] = {x, y};
        if (uboMembers.count(name)) { setUBOField(name, v, 8); return; }
        glUniform2f(uniform(name), x, y);
    }

    void setVec3(const std::string& name, float x, float y, float z) override {
        float v[3] = {x, y, z};
        if (uboMembers.count(name)) { setUBOField(name, v, 12); return; }
        glUniform3f(uniform(name), x, y, z);
    }

    void setVec4(const std::string& name, float x, float y, float z, float w) override {
        float v[4] = {x, y, z, w};
        if (uboMembers.count(name)) { setUBOField(name, v, 16); return; }
        glUniform4f(uniform(name), x, y, z, w);
    }

    [[nodiscard]] GLuint id() const { return program; }

private:
    GLint uniform(const std::string& name) {
        auto it = uniformCache.find(name);
        if (it != uniformCache.end()) {
            return it->second;
        }
        GLint loc = glGetUniformLocation(program, name.c_str());
        uniformCache[name] = loc;
        return loc;
    }

    static std::string loadTextFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[ERROR] Failed to open file: " << path << std::endl;
            throw std::runtime_error("Failed to open shader file: " + path);
        }
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    static GLuint compile(GLenum type, const std::string& source) {
        GLuint id = glCreateShader(type);

        std::string processedSource = source;
#ifdef __EMSCRIPTEN__
        // Replace "#version 330 core" with "#version 300 es" and add precision qualifiers
        size_t versionPos = processedSource.find("#version");
        if (versionPos != std::string::npos) {
            size_t endLine = processedSource.find('\n', versionPos);
            if (endLine != std::string::npos) {
                processedSource.replace(versionPos, endLine - versionPos, "#version 300 es\nprecision highp float;\nprecision highp int;");
            }
        }
#endif

        const char* src = processedSource.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);

        GLint compiled = 0;
        glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint logLength = 0;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(static_cast<size_t>(logLength));
            glGetShaderInfoLog(id, logLength, nullptr, log.data());
            std::cerr << "[ERROR] Compilation failed: " << log.data() << std::endl;
            glDeleteShader(id);
            throw std::runtime_error("Shader compilation failed");
        }
        return id;
    }

    void link(GLuint vs, GLuint fs) {
        program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        GLint linked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint logLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(static_cast<size_t>(logLength));
            glGetProgramInfoLog(program, logLength, nullptr, log.data());
            std::cerr << "[ERROR] Link failed: " << log.data() << std::endl;

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

        discoverUBOs();
    }

    void discoverUBOs() {
        GLint numBlocks = 0;
        glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &numBlocks);
        if (numBlocks == 0) return;

        uboBlocks.resize(static_cast<size_t>(numBlocks));

        for (int b = 0; b < numBlocks; ++b) {
            GLint blockSize = 0;
            glGetActiveUniformBlockiv(program, static_cast<GLuint>(b), GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

            GLuint bindingPoint = static_cast<GLuint>(b);
            glUniformBlockBinding(program, static_cast<GLuint>(b), bindingPoint);

            GLuint ubo = 0;
            glGenBuffers(1, &ubo);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo);
            glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            UBOBlock& block = uboBlocks[static_cast<size_t>(b)];
            block.bufferObject = ubo;
            block.bindingPoint = bindingPoint;
            block.size = static_cast<GLsizeiptr>(blockSize);
            block.shadowBuffer.assign(static_cast<size_t>(blockSize), 0);
        }

        GLint numUniforms = 0;
        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);

        for (GLuint u = 0; u < static_cast<GLuint>(numUniforms); ++u) {
            GLint blockIndex = 0;
            glGetActiveUniformsiv(program, 1, &u, GL_UNIFORM_BLOCK_INDEX, &blockIndex);
            if (blockIndex == -1) continue;

            GLint offset = 0;
            glGetActiveUniformsiv(program, 1, &u, GL_UNIFORM_OFFSET, &offset);

            GLint arraySize = 0;
            glGetActiveUniformsiv(program, 1, &u, GL_UNIFORM_SIZE, &arraySize);
            GLint type = 0;
            glGetActiveUniformsiv(program, 1, &u, GL_UNIFORM_TYPE, &type);

            GLint byteSize = 4;
            switch (type) {
                case GL_FLOAT:       byteSize = 4;  break;
                case GL_INT:
                case GL_BOOL:
                case GL_UNSIGNED_INT:byteSize = 4;  break;
                case GL_FLOAT_VEC2:  byteSize = 8;  break;
                case GL_FLOAT_VEC3:  byteSize = 12; break;
                case GL_FLOAT_VEC4:  byteSize = 16; break;
                case GL_INT_VEC2:    byteSize = 8;  break;
                case GL_INT_VEC3:    byteSize = 12; break;
                case GL_INT_VEC4:    byteSize = 16; break;
                case GL_FLOAT_MAT3:  byteSize = 36; break;
                case GL_FLOAT_MAT4:  byteSize = 64; break;
                default:             byteSize = 4;  break;
            }

            char nameBuf[256];
            GLsizei nameLen = 0;
            glGetActiveUniform(program, u, 255, &nameLen, &arraySize, reinterpret_cast<GLenum*>(&type), nameBuf);
            std::string fullName(nameBuf, static_cast<size_t>(nameLen));

            size_t dot = fullName.find('.');
            std::string memberName = (dot != std::string::npos) ? fullName.substr(dot + 1) : fullName;

            size_t underscore = memberName.rfind('_');
            if (underscore != std::string::npos && underscore + 1 < memberName.size()) {
                bool allDigits = true;
                for (size_t i = underscore + 1; i < memberName.size(); ++i) {
                    if (memberName[i] < '0' || memberName[i] > '9') { allDigits = false; break; }
                }
                if (allDigits) memberName = memberName.substr(0, underscore);
            }

            UBOMember member;
            member.blockIndex = blockIndex;
            member.offset = offset;
            member.size = byteSize;
            uboMembers[memberName] = member;
        }
    }

    void setUBOField(const std::string& name, const void* data, GLint size) {
        auto it = uboMembers.find(name);
        if (it == uboMembers.end()) return;

        const UBOMember& m = it->second;
        UBOBlock& block = uboBlocks[static_cast<size_t>(m.blockIndex)];

        GLint copyBytes = std::min(size, m.size);
        std::memcpy(block.shadowBuffer.data() + m.offset, data, static_cast<size_t>(copyBytes));
    }

    struct UBOBlock {
        GLuint bufferObject = 0;
        GLuint bindingPoint = 0;
        GLsizeiptr size = 0;
        std::vector<uint8_t> shadowBuffer;
    };

    struct UBOMember {
        int blockIndex = -1;
        GLint offset = 0;
        GLint size = 0;
    };

    GLuint program = 0;
    std::unordered_map<std::string, GLint> uniformCache;
    std::vector<UBOBlock> uboBlocks;
    std::unordered_map<std::string, UBOMember> uboMembers;
};

} // namespace GL
