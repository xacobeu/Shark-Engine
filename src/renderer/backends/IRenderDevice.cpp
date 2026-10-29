module shark:renderer.backends.IRenderDevice.impl;

import std;

import :renderer.backends.IRenderDevice;

import :renderer.backends.opengl.RenderDevice;

namespace shark {

std::unique_ptr<IRenderDevice> IRenderDevice::create(GraphicsAPI api) {
    if (api == GraphicsAPI::OpenGL) {
        return std::make_unique<GL::RenderDevice>();
    }
    throw std::runtime_error("Unsupported Graphics API!");
}

} // namespace shark
