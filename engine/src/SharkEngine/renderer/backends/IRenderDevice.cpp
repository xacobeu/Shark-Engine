#include <SharkEngine/renderer/backends/IRenderDevice.hpp>
#include <SharkEngine/renderer/backends/opengl/RenderDevice.hpp>
#include <stdexcept>

std::unique_ptr<IRenderDevice> IRenderDevice::create(GraphicsAPI api) {
    switch (api) {
        case GraphicsAPI::OpenGL:
            return std::make_unique<GL::RenderDevice>();
        default:
            throw std::runtime_error("Unsupported Graphics API!");
    }
}
