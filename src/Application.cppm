module;

#include <GLFW/glfw3.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

export module shark:Application;

import std;

import :Input;
import :Layer;
import :Window;
import :renderer.Renderer;
import :renderer.backends.GraphicsAPI;
import :renderer.backends.IRenderDevice;

export namespace shark {
struct Config {
    int         width  = 800;
    int         height = 600;
    const char* title  = "Application";
    bool        vsync  = true;
    GraphicsAPI api    = GraphicsAPI::OpenGL;

    float fixedTimestep = 1.0f / 60.0f;
    int   maxFixedSteps = 4;
};

class Application {
public:
    explicit Application(const Config& config);
    ~Application() = default;

    // Disable copy
    Application(const Application&)                    = delete ("owns the window and GL context");
    auto operator=(const Application&) -> Application& = delete ("owns the window and GL context");

    // Disable move
    Application(Application&&)                    = delete ("layers point back to their Application");
    auto operator=(Application&&) -> Application& = delete ("layers point back to their Application");

    void run();
    void runStep();
    void close();

    const std::vector<std::unique_ptr<Layer>>& getLayers() const;

    inline Window::Events& getWindowEvents() { return window.getEvents(); }
    inline Renderer&       getRenderer() { return renderer; }
    inline void            getWindowDimensions(int& width, int& height) const { window.getDimensions(width, height); }
    inline int             getWidth() const { return config.width; }
    inline int             getHeight() const { return config.height; }

    inline float getElapsedTime() const { return elapsedTime; }
    inline float getDeltaTime() const { return deltaTime; }

    inline float getFixedAlpha() const { return fixedAlpha; }
    inline float getFixedTimestep() const { return config.fixedTimestep; }

    void updateTime(float now);

    inline double getCursorX() const { return window.getCursorX(); }
    inline double getCursorY() const { return window.getCursorY(); }

    inline bool        isKeyPressed(int key) const { return window.isKeyPressed(key); }
    inline bool        isKeyPressed(Key key) const { return window.isKeyPressed(key); }
    inline bool        isMouseButtonPressed(int button) const { return window.isMouseButtonPressed(button); }
    inline bool        isMouseButtonPressed(MouseButton button) const { return window.isMouseButtonPressed(button); }
    inline static bool isMouseCaptured() noexcept { return Window::isMouseCaptured(); }
    inline static bool isKeyboardCaptured() noexcept { return Window::isKeyboardCaptured(); }
    inline double      getScrollX() const { return window.getScrollX(); }
    inline double      getScrollY() const { return window.getScrollY(); }

    inline void                 setScene(Scene* scene) { activeScene = scene; }
    [[nodiscard]] inline Scene* getScene() const { return activeScene; }

    [[nodiscard]] inline IRenderDevice* getDevice() const { return device.get(); }

private:
    Scene* activeScene = nullptr;
    Config config;

    // Destroyed bottom-up: GPU caches, then the device, then the window that owns the surface/context.
    Window                         window;
    std::unique_ptr<IRenderDevice> device;
    Renderer                       renderer;

    std::vector<std::unique_ptr<Layer>> layers;

    float elapsedTime   = 0.0f;
    float deltaTime     = 0.0f;
    float lastFrameTime = 0.0f;

    float fixedAccumulator = 0.0f;
    float fixedAlpha       = 0.0f;

    void runFixedSteps();

public:
    template <typename T>
        requires std::is_base_of_v<Layer, T>
    void push(std::unique_ptr<T> layer) {
        if constexpr (UniqueLayer<T>) {
            if (getLayer<T>() != nullptr) {
                throw std::runtime_error("Duplicate unique layer was added to application.");
            }
        }

        layer->app = this;
        layer->onAttach();
        layers.emplace_back(std::move(layer));
    }

    template <typename T> T* getLayer() {
        for (auto& layer : layers) {
            if (auto ptr = dynamic_cast<T*>(layer.get())) {
                return ptr;
            }
        }

        return nullptr;
    }
};

} // namespace shark

namespace shark {

#ifdef __EMSCRIPTEN__
static void emscriptenLoop(void* arg) {
    Application* app = static_cast<Application*>(arg);
    app->runStep();
}
#endif

Application::Application(const Config& config)
    : config(config),
      window(config.width, config.height, config.title, config.api),
      device(IRenderDevice::create(config.api, window)) {
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

    device->beginImGuiFrame();
    Window::beginImGuiFrame();

    if (activeScene != nullptr) {
        if (auto* camera = activeScene->getActiveCamera()) {
            camera->update(this, deltaTime);
        }
    }

    runFixedSteps();

    for (auto& layer : layers) {
        layer->onUpdate(deltaTime);
    }

    renderer.begin(config.width, config.height);

    for (auto& layer : layers) {
        layer->onRender(renderer);
    }

    Window::endImGuiFrame();
    renderer.end();
    window.resetScroll();
}

void Application::runFixedSteps() {
    const float step     = config.fixedTimestep;
    const int   maxSteps = std::max(config.maxFixedSteps, 1);

    fixedAccumulator += std::min(deltaTime, step * static_cast<float>(maxSteps));

    const auto budgetStart = std::chrono::steady_clock::now();
    const auto budget      = std::chrono::duration<float>(step);

    int  steps      = 0;
    bool fellBehind = false;
    while (fixedAccumulator >= step) {
        if (steps >= maxSteps || (steps > 0 && std::chrono::steady_clock::now() - budgetStart >= budget)) {
            fellBehind = true;
            break;
        }

        for (auto& layer : layers) {
            layer->onFixedUpdate(step);
        }
        fixedAccumulator -= step;
        ++steps;
    }

    if (fellBehind) {
        fixedAccumulator = std::min(fixedAccumulator, step);
    }

    fixedAlpha = std::clamp(fixedAccumulator / step, 0.0f, 1.0f);
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

} // namespace shark
