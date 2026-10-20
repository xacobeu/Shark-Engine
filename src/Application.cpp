#include "Application.hpp"

#include <GLFW/glfw3.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static void emscriptenLoop(void* arg) {
    Application* app = static_cast<Application*>(arg);
    app->runStep();
}
#endif

#include "renderer/backends/IRenderDevice.hpp"

Application::Application(const Config& config) : config(config), window(config.width, config.height, config.title) {
    device = IRenderDevice::create(config.api);
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
#ifdef __EMSCRIPTEN__
    if (window.shouldClose()) {
        emscripten_cancel_main_loop();
        emscripten_force_exit(0);
        return;
    }
#endif

    updateTime(static_cast<float>(glfwGetTime()));

    window.pollEvents();
    window.getDimensions(config.width, config.height);

    Window::beginImGuiFrame();

    if (activeScene != nullptr) {
        if (auto* camera = activeScene->getActiveCamera()) {
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

    Window::endImGuiFrame();
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
