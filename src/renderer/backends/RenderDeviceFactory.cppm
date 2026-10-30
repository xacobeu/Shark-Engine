module shark:renderer.backends.RenderDeviceFactory;

import std;

import :Window;
import :renderer.backends.GraphicsAPI;
import :renderer.backends.IRenderDevice;

import :renderer.backends.opengl.RenderDevice;

namespace shark {

std::unique_ptr<IRenderDevice> IRenderDevice::create(GraphicsAPI api, Window& window) {
    if (api == GraphicsAPI::OpenGL) {
        return std::make_unique<GL::RenderDevice>(window);
    }
    throw std::runtime_error("Unsupported Graphics API!");
}

} // namespace shark
