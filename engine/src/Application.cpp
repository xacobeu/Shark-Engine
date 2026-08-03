#include <SharkEngine/Application.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static void emscriptenLoop(void* arg) {
    Application* app = static_cast<Application*>(arg);
    app->runStep();
}
#endif

#include <SharkEngine/renderer/backends/IRenderDevice.hpp>

Application::Application(const Config& config) : window(config.width, config.height, config.title) {
    device = IRenderDevice::create(GraphicsAPI::OpenGL);
    renderer.init(device.get());
    device->setVSync(config.vsync);
}

void Application::run() {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(emscriptenLoop, this, 0, 1);
#else
    while (!window.shouldClose()) {
        runStep();
    }
#endif
}

void Application::runStep() {
    updateTime(static_cast<float>(glfwGetTime()));

    window.pollEvents();
    window.getDimensions(config.width, config.height);

    window.beginImGuiFrame();

    if (activeScene) {
        if (auto camera = activeScene->getActiveCamera()) {
            camera->update(this, deltaTime);
        }
    }

    for (auto& layer : layers) {
        layer->onUpdate(deltaTime);
    }

    renderer.begin(config.width, config.height);

    for (auto& layer : layers) {
        layer->onRender(renderer);
    }

    window.endImGuiFrame();
    window.swapBuffers();
    window.resetScroll();
}

void Application::close() {
    window.close();
}

const std::vector<std::unique_ptr<Layer>>& Application::getLayers() const {
    return layers;
}

void Application::updateTime(float now) {
    deltaTime = now - lastFrameTime;
    elapsedTime += deltaTime;
    lastFrameTime = now;
}
