#include "renderer/backends/IRenderDevice.hpp"

#include <stdexcept>

#include "renderer/backends/opengl/RenderDevice.hpp"

std::unique_ptr<IRenderDevice> IRenderDevice::create(GraphicsAPI api) {
    if (api == GraphicsAPI::OpenGL) {
        return std::make_unique<GL::RenderDevice>();
    }
    throw std::runtime_error("Unsupported Graphics API!");
}