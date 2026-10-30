module;

#ifdef SHARK_NO_GLAD
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

module shark:renderer.backends.opengl.Pipeline;

import std;

import :renderer.backends.IPipeline;
import :util.ResourcePath;

namespace shark::GL {

class Pipeline final : public IPipeline {
public:
    explicit Pipeline(const PipelineDesc& desc);
    ~Pipeline() override;

    Pipeline(const Pipeline&)            = delete ("would double-delete the GL object; move it");
    Pipeline& operator=(const Pipeline&) = delete ("would double-delete the GL object; move it");

    Pipeline(Pipeline&&) noexcept            = delete ("the device holds pointers to bound pipelines");
    Pipeline& operator=(Pipeline&&) noexcept = delete ("the device holds pointers to bound pipelines");

    [[nodiscard]] inline GLuint                              id() const { return program; }
    [[nodiscard]] inline std::span<const VertexBufferLayout> vertexBuffers() const { return layouts; }
    [[nodiscard]] inline bool                                depthTest() const { return depthTestEnabled; }
    [[nodiscard]] inline bool                                depthWrite() const { return depthWriteEnabled; }

private:
    void               bindResources(std::span<const BindingDesc> bindings) const;
    static std::string loadTextFile(const std::string& path);
    static GLuint      compile(GLenum type, const std::string& source);

    GLuint                          program = 0;
    std::vector<VertexBufferLayout> layouts;
    bool                            depthTestEnabled  = true;
    bool                            depthWriteEnabled = true;
};

} // namespace shark::GL

namespace shark::GL {

Pipeline::Pipeline(const PipelineDesc& desc)
    : program(glCreateProgram()),
      layouts(desc.vertexBuffers),
      depthTestEnabled(desc.depthTest),
      depthWriteEnabled(desc.depthWrite) {
    GLuint vs = compile(GL_VERTEX_SHADER, loadTextFile(desc.shader + ".vert.glsl"));
    GLuint fs = compile(GL_FRAGMENT_SHADER, loadTextFile(desc.shader + ".frag.glsl"));

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

    bindResources(desc.bindings);
}

Pipeline::~Pipeline() {
    if (program != 0) {
        glDeleteProgram(program);
    }
}

// GLSL 330 / ESSL 300 can't declare binding numbers, so assign them here. Blocks or samplers the compiler
// optimized out are skipped.
void Pipeline::bindResources(std::span<const BindingDesc> bindings) const {
    glUseProgram(program);
    for (const auto& binding : bindings) {
        if (binding.type == BindingType::UniformBuffer) {
            const GLuint index = glGetUniformBlockIndex(program, binding.name.c_str());
            if (index != GL_INVALID_INDEX) {
                glUniformBlockBinding(program, index, binding.binding);
            }
        } else {
            const GLint location = glGetUniformLocation(program, binding.name.c_str());
            if (location != -1) {
                glUniform1i(location, static_cast<GLint>(binding.binding));
            }
        }
    }
    glUseProgram(0);
}

std::string Pipeline::loadTextFile(const std::string& path) {
    std::ifstream file(resolveResourcePath(path));
    if (!file.is_open()) {
        std::println(std::cerr, "[ERROR] Failed to open file: {}", path);
        throw std::runtime_error("Failed to open shader file: " + path);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GLuint Pipeline::compile(GLenum type, const std::string& source) {
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
