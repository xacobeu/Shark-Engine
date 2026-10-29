#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include "Window.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/backends/IRenderDevice.hpp"

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

    Window::Events& getWindowEvents() { return window.getEvents(); }
    Renderer&       getRenderer() { return renderer; }
    void            getWindowDimensions(int& width, int& height) const { window.getDimensions(width, height); }
    int             getWidth() const { return config.width; }
    int             getHeight() const { return config.height; }

    float getElapsedTime() const { return elapsedTime; }
    float getDeltaTime() const { return deltaTime; }

    float getFixedAlpha() const { return fixedAlpha; }
    float getFixedTimestep() const { return config.fixedTimestep; }

    void updateTime(float now);

    double getCursorX() const { return window.getCursorX(); }
    double getCursorY() const { return window.getCursorY(); }

    bool   isKeyPressed(int key) const { return window.isKeyPressed(key); }
    bool   isMouseButtonPressed(int button) const { return window.isMouseButtonPressed(button); }
    double getScrollX() const { return window.getScrollX(); }
    double getScrollY() const { return window.getScrollY(); }

    void                 setScene(Scene* scene) { activeScene = scene; }
    [[nodiscard]] Scene* getScene() const { return activeScene; }

    [[nodiscard]] IRenderDevice* getDevice() const { return device.get(); }

private:
    Scene* activeScene = nullptr;
    Config config;

    std::unique_ptr<IRenderDevice> device;
    Renderer                       renderer;
    Window                         window;

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
