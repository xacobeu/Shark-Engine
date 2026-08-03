#pragma once

#include <SharkEngine/renderer/Renderer.hpp>
#include <SharkEngine/Window.hpp>

#include <memory>
#include <vector>
#include <stdexcept>

struct Config {
    int width = 800;
    int height = 600;
    const char* title = "Application";
    bool vsync = true;
};

class Application {
public:

    Application(const Config& config);
    ~Application() = default;

    void run();
    void runStep();
    void close();

    const std::vector<std::unique_ptr<Layer>>& getLayers() const;

    Window::Events& getWindowEvents() { return window.events; }
    Renderer& getRenderer() { return renderer; }
    void getWindowDimensions(int& width, int& height) const { window.getDimensions(width, height); }
    int getWidth() const { return config.width; }
    int getHeight() const { return config.height; }

    float getElapsedTime() const { return elapsedTime; }
    float getDeltaTime() const { return deltaTime; }

    void updateTime(float now);

    double getCursorX() const { return window.getCursorX(); }
    double getCursorY() const { return window.getCursorY(); }

    bool isKeyPressed(int key) const { return window.isKeyPressed(key); }
    bool isMouseButtonPressed(int button) const { return window.isMouseButtonPressed(button); }
    double getScrollX() const { return window.getScrollX(); }
    double getScrollY() const { return window.getScrollY(); }

    void setScene(Scene* scene) { activeScene = scene; }
    [[nodiscard]] Scene* getScene() const { return activeScene; }

private:
    Scene* activeScene = nullptr;
    Config config;

    Renderer renderer;
    Window window;

    std::vector<std::unique_ptr<Layer>> layers;

    float elapsedTime = 0.0f;
    float deltaTime = 0.0f;
    float lastFrameTime = 0.0f;

public:

    template<typename T>
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

    template<typename T>
    T* getLayer() {

        for (auto& layer : layers) {
            if (auto ptr = dynamic_cast<T*>(layer.get())) {
                return ptr;
            }
        }

        return nullptr;
    }
};
