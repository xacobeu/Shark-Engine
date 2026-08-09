#include <SharkEngine/renderer/backends/opengl/Shader.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace GL {

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath) {
    std::string vertexSource = loadTextFile(vertexPath);
    std::string fragmentSource = loadTextFile(fragmentPath);

    GLuint vs = compile(GL_VERTEX_SHADER, vertexSource);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fragmentSource);

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
}

Shader::Shader(SourceStringTag, const std::string &vertexSource, const std::string &fragmentSource) {
    GLuint vs = compile(GL_VERTEX_SHADER, vertexSource);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fragmentSource);

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
}

Shader::~Shader() {
    if (program != 0) {
        glDeleteProgram(program);
    }
}

void Shader::bind() {
    glUseProgram(program);
}

void Shader::unbind() {
    glUseProgram(0);
}

GLint Shader::uniform(const std::string &name) {
    auto it = uniformCache.find(name);
    if (it != uniformCache.end()) {
        return it->second;
    }
    GLint loc = glGetUniformLocation(program, name.c_str());
    uniformCache[name] = loc;
    return loc;
}

void Shader::setMat4(const std::string &name, const float *matrixData) {
    glUniformMatrix4fv(uniform(name), 1, GL_FALSE, matrixData);
}

void Shader::setInt(const std::string &name, int value) {
    glUniform1i(uniform(name), value);
}

void Shader::setFloat(const std::string &name, float value) {
    glUniform1f(uniform(name), value);
}

void Shader::setVec2(const std::string &name, float x, float y) {
    glUniform2f(uniform(name), x, y);
}

void Shader::setVec3(const std::string &name, float x, float y, float z) {
    glUniform3f(uniform(name), x, y, z);
}

void Shader::setVec4(const std::string &name, float x, float y, float z, float w) {
    glUniform4f(uniform(name), x, y, z, w);
}

std::string Shader::loadTextFile(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open() && !path.empty() && path[0] != '/') {
        file.open("/" + path);
    }
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open file: " << path << std::endl;
        throw std::runtime_error("Failed to open shader file: " + path);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GLuint Shader::compile(GLenum type, const std::string &source) {
    std::string processedSource = source;
#ifdef __EMSCRIPTEN__
    size_t verPos = processedSource.find("#version 330 core");
    if (verPos != std::string::npos) {
        processedSource.replace(verPos, 17, "#version 300 es");
    }
    if (processedSource.find("precision ") == std::string::npos) {
        size_t versionEnd = processedSource.find('\n');
        if (versionEnd != std::string::npos) {
            processedSource.insert(versionEnd + 1, "precision highp float;\nprecision highp int;\n");
        }
    }
#endif
    GLuint      id = glCreateShader(type);
    const char *src = processedSource.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    GLint compiled = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint logLength = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<size_t>(logLength > 0 ? logLength : 1024));
        glGetShaderInfoLog(id, static_cast<GLsizei>(log.size()), nullptr, log.data());
        std::cerr << "[ERROR] Shader compilation failed: " << log.data() << std::endl;
        glDeleteShader(id);
        throw std::runtime_error(std::string("Shader compilation failed: ") + log.data());
    }
    return id;
}

} // namespace GL
