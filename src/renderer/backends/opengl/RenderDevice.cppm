module;

#ifdef SHARK_NO_GLAD
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

module shark:renderer.backends.opengl.RenderDevice;

import std;

import :Window;
import :renderer.backends.IBuffer;
import :renderer.backends.IPipeline;
import :renderer.backends.IRenderDevice;
import :renderer.backends.ITexture;
import :renderer.backends.opengl.Buffer;
import :renderer.backends.opengl.Object;
import :renderer.backends.opengl.Pipeline;
import :renderer.backends.opengl.Texture;
import :renderer.backends.opengl.UniformRing;
import :util.Vector;

namespace shark::GL {

class RenderDevice final : public IRenderDevice {
public:
    explicit RenderDevice(Window& window);
    ~RenderDevice() override;

    RenderDevice(RenderDevice&&)            = delete ("ImGui's OpenGL backend is tied to this device");
    RenderDevice& operator=(RenderDevice&&) = delete ("ImGui's OpenGL backend is tied to this device");

    std::shared_ptr<IBuffer>   createBuffer(BufferUsage usage, const void* data, std::size_t size) override;
    std::shared_ptr<ITexture>  createTexture(const std::string& filepath) override;
    std::shared_ptr<IPipeline> createPipeline(const PipelineDesc& desc) override;

    void beginImGuiFrame() override;
    void beginFrame(int width, int height, const Vector4& clearColor) override;
    void endFrame() override;

    void setViewport(int x, int y, int width, int height) override;
    void setPipeline(const IPipeline& pipeline) override;
    void setVertexBuffer(std::uint32_t slot, const IBuffer& buffer) override;
    void setIndexBuffer(const IBuffer& buffer) override;
    void setUniformData(std::uint32_t binding, const void* data, std::size_t size) override;
    void setTexture(std::uint32_t binding, const ITexture* texture) override;

    void draw(std::uint32_t vertexCount, std::uint32_t instanceCount) override;
    void drawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount) override;

    void setVSync(bool enabled) override;

private:
    static constexpr std::size_t MAX_VERTEX_BUFFERS = 4;

    void flushVertexState();

    GLFWwindow*           window = nullptr;
    GL::VertexArrayObject vao;
    GL::UniformRing       uniforms;

    // Attribute pointers are re-specified on the next draw whenever the pipeline or a vertex buffer changes,
    // since GL 3.3 / ES 3.0 have no separate vertex buffer bindings.
    const GL::Pipeline*                               pipeline = nullptr;
    std::array<const GL::Buffer*, MAX_VERTEX_BUFFERS> vertexBuffers{};
    std::array<std::uint64_t, MAX_VERTEX_BUFFERS>     vertexBufferSerials{};
    const GL::Buffer*                                 indexBuffer       = nullptr;
    std::uint32_t                                     enabledAttributes = 0;
    bool                                              vertexStateDirty  = true;
};

} // namespace shark::GL

namespace shark::GL {

namespace {

[[nodiscard]] constexpr GLint componentCount(VertexFormat format) {
    switch (format) {
    case VertexFormat::Float:
        return 1;
    case VertexFormat::Float2:
        return 2;
    case VertexFormat::Float3:
        return 3;
    case VertexFormat::Float4:
        return 4;
    }
    return 4;
}

} // namespace

RenderDevice::RenderDevice(Window& window) : window(window.getNativeHandle()) {
    glfwMakeContextCurrent(this->window);

#ifndef __EMSCRIPTEN__
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
#endif

#ifdef __EMSCRIPTEN__
    ImGui_ImplOpenGL3_Init("#version 300 es");
#else
    ImGui_ImplOpenGL3_Init("#version 330");
#endif

    vao.create();
}

RenderDevice::~RenderDevice() {
    ImGui_ImplOpenGL3_Shutdown();
}

std::shared_ptr<IBuffer> RenderDevice::createBuffer(BufferUsage usage, const void* data, std::size_t size) {
    return std::make_shared<GL::Buffer>(usage, data, size);
}

std::shared_ptr<ITexture> RenderDevice::createTexture(const std::string& filepath) {
    return std::make_shared<GL::Texture>(filepath);
}

std::shared_ptr<IPipeline> RenderDevice::createPipeline(const PipelineDesc& desc) {
    return std::make_shared<GL::Pipeline>(desc);
}

void RenderDevice::beginImGuiFrame() {
    ImGui_ImplOpenGL3_NewFrame();
}

void RenderDevice::beginFrame(int width, int height, const Vector4& clearColor) {
    vao.bind();
    pipeline = nullptr;
    vertexBuffers.fill(nullptr);
    vertexBufferSerials.fill(0);
    indexBuffer      = nullptr;
    vertexStateDirty = true;
    uniforms.reset();

    glViewport(0, 0, width, height);
    glDepthMask(GL_TRUE); // a pipeline without depth writes would otherwise block the depth clear
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderDevice::endFrame() {
    if (ImDrawData* drawData = ImGui::GetDrawData()) {
        ImGui_ImplOpenGL3_RenderDrawData(drawData);
    }

    if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0) {
        GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backupCurrentContext);
    }

    glfwSwapBuffers(window);
}

void RenderDevice::setViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void RenderDevice::setPipeline(const IPipeline& newPipeline) {
    const auto& glPipeline = dynamic_cast<const GL::Pipeline&>(newPipeline);
    if (&glPipeline == pipeline) {
        return;
    }
    pipeline         = &glPipeline;
    vertexStateDirty = true;

    glUseProgram(pipeline->id());
    if (pipeline->depthTest()) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    glDepthMask(pipeline->depthWrite() ? GL_TRUE : GL_FALSE);
}

void RenderDevice::setVertexBuffer(std::uint32_t slot, const IBuffer& buffer) {
    if (slot >= MAX_VERTEX_BUFFERS) {
        throw std::out_of_range("Vertex buffer slot out of range");
    }
    const auto& glBuffer = dynamic_cast<const GL::Buffer&>(buffer);
    // Compared by serial: a buffer freed mid-frame could be replaced by a new one at the same address.
    if (vertexBufferSerials[slot] != glBuffer.serial()) {
        vertexBuffers[slot]       = &glBuffer;
        vertexBufferSerials[slot] = glBuffer.serial();
        vertexStateDirty          = true;
    }
}

void RenderDevice::setIndexBuffer(const IBuffer& buffer) {
    indexBuffer = &dynamic_cast<const GL::Buffer&>(buffer);
}

void RenderDevice::setUniformData(std::uint32_t binding, const void* data, std::size_t size) {
    uniforms.push(binding, data, size);
}

void RenderDevice::setTexture(std::uint32_t binding, const ITexture* texture) {
    glActiveTexture(GL_TEXTURE0 + binding);
    glBindTexture(GL_TEXTURE_2D, texture != nullptr ? dynamic_cast<const GL::Texture&>(*texture).id() : 0);
}

void RenderDevice::flushVertexState() {
    if (pipeline == nullptr) {
        throw std::logic_error("Draw issued without a pipeline");
    }
    if (!vertexStateDirty) {
        return;
    }

    std::uint32_t wanted  = 0;
    const auto    layouts = pipeline->vertexBuffers();
    for (std::size_t slot = 0; slot < layouts.size(); ++slot) {
        const auto& layout = layouts[slot];
        const auto* buffer = slot < MAX_VERTEX_BUFFERS ? vertexBuffers[slot] : nullptr;
        if (buffer == nullptr) {
            throw std::logic_error("Pipeline expects a vertex buffer in slot " + std::to_string(slot));
        }

        glBindBuffer(GL_ARRAY_BUFFER, buffer->id());
        const GLuint divisor = layout.stepMode == VertexStepMode::Instance ? 1 : 0;
        for (const auto& attribute : layout.attributes) {
            glEnableVertexAttribArray(attribute.location);
            glVertexAttribPointer(
                attribute.location, componentCount(attribute.format), GL_FLOAT, GL_FALSE,
                static_cast<GLsizei>(layout.stride), bufferOffset(attribute.offset)
            );
            glVertexAttribDivisor(attribute.location, divisor);
            wanted |= 1u << attribute.location;
        }
    }

    for (std::uint32_t stale = enabledAttributes & ~wanted; stale != 0; stale &= stale - 1) {
        glDisableVertexAttribArray(static_cast<GLuint>(std::countr_zero(stale)));
    }
    enabledAttributes = wanted;
    vertexStateDirty  = false;
}

void RenderDevice::draw(std::uint32_t vertexCount, std::uint32_t instanceCount) {
    if (vertexCount == 0 || instanceCount == 0) {
        return;
    }
    flushVertexState();
    glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount), static_cast<GLsizei>(instanceCount));
}

void RenderDevice::drawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount) {
    if (indexCount == 0 || instanceCount == 0) {
        return;
    }
    if (indexBuffer == nullptr) {
        throw std::logic_error("Indexed draw issued without an index buffer");
    }
    flushVertexState();
    // Rebound every time: creating or updating an index buffer mid-frame replaces the VAO's element binding.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->id());
    glDrawElementsInstanced(
        GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(instanceCount)
    );
}

void RenderDevice::setVSync(bool enabled) {
#ifndef __EMSCRIPTEN__
    glfwSwapInterval(enabled ? 1 : 0);
#else
    (void)enabled;
#endif
}

} // namespace shark::GL
