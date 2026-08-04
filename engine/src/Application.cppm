module;
#include <memory>
#include <vector>
#include <stdexcept>
#ifdef SHARK_NO_GLAD
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cmath>
#include <numbers>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

export module SharkEngine:Application;
import :Layer;
import :Window;
import :Renderer;
import :RenderDeviceAbstractions;
import :Scene;
import :SceneNodes;
import :Math;

#ifdef __EMSCRIPTEN__
static void emscriptenLoop(void* arg);
#endif

export struct Config {
    int width = 800;
    int height = 600;
    const char* title = "Application";
    bool vsync = true;
    GraphicsAPI api = GraphicsAPI::OpenGL;
};

export class Application {
public:
    Application(const Config& config) : window(config.width, config.height, config.title, config.api) {
        device = IRenderDevice::create(config.api);
        renderer.init(device.get());
        device->setVSync(config.vsync);
    }

    ~Application() = default;

    void run() {
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop_arg(emscriptenLoop, this, 0, 1);
#else
        while (!window.shouldClose()) {
            runStep();
        }
#endif
    }

    void runStep() {
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

        device->endFrame();

        window.endImGuiFrame();
        window.swapBuffers();
        window.resetScroll();
    }

    void close() {
        window.close();
    }

    const std::vector<std::unique_ptr<Layer>>& getLayers() const {
        return layers;
    }

    Window::Events& getWindowEvents() { return window.events; }
    Renderer& getRenderer() { return renderer; }
    void getWindowDimensions(int& width, int& height) const { window.getDimensions(width, height); }
    int getWidth() const { return config.width; }
    int getHeight() const { return config.height; }

    float getElapsedTime() const { return elapsedTime; }
    float getDeltaTime() const { return deltaTime; }

    void updateTime(float now) {
        deltaTime = now - lastFrameTime;
        elapsedTime += deltaTime;
        lastFrameTime = now;
    }

    double getCursorX() const { return window.getCursorX(); }
    double getCursorY() const { return window.getCursorY(); }

    bool isKeyPressed(int key) const { return window.isKeyPressed(key); }
    bool isMouseButtonPressed(int button) const { return window.isMouseButtonPressed(button); }
    double getScrollX() const { return window.getScrollX(); }
    double getScrollY() const { return window.getScrollY(); }

    void setScene(Scene* scene) { activeScene = scene; }
    [[nodiscard]] Scene* getScene() const { return activeScene; }

    [[nodiscard]] IRenderDevice* getDevice() const { return device.get(); }

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

private:
    Scene* activeScene = nullptr;
    Config config;

    std::unique_ptr<IRenderDevice> device;
    Renderer renderer;
    Window window;

    std::vector<std::unique_ptr<Layer>> layers;

    float elapsedTime = 0.0f;
    float deltaTime = 0.0f;
    float lastFrameTime = 0.0f;
};

#ifdef __EMSCRIPTEN__
static void emscriptenLoop(void* arg) {
    Application* app = static_cast<Application*>(arg);
    app->runStep();
}
#endif

// Now implement Camera, FlyingCamera, and OrbitCamera out-of-line update methods

void Camera::update(Application* /*app*/, float /*deltaTime*/) {}

void FlyingCamera::update(Application* app, float deltaTime) {
    double mouseX = app->getCursorX();
    double mouseY = app->getCursorY();

    if (app->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) || app->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (!isDragging) {
            isDragging = true;
            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }

        double dx = mouseX - lastMouseX;
        double dy = mouseY - lastMouseY;
        Vector3 rot = getRotation();
        rot.y += static_cast<float>(dx) * 0.15f;
        rot.x -= static_cast<float>(dy) * 0.15f;

        if (rot.x > 89.0f) rot.x = 89.0f;
        if (rot.x < -89.0f) rot.x = -89.0f;

        setRotation(rot);

        lastMouseX = mouseX;
        lastMouseY = mouseY;
    } else {
        isDragging = false;
    }

    float speed = 5.0f * deltaTime;

    float pitchRad = transform.rotationDeg.x * std::numbers::pi_v<float> / 180.0f;
    float yawRad = transform.rotationDeg.y * std::numbers::pi_v<float> / 180.0f;

    Vector3 direction = {
        std::cos(pitchRad) * std::cos(yawRad),
        std::sin(pitchRad),
        std::cos(pitchRad) * std::sin(yawRad)
    };

    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (len > 0.0f) {
        direction.x /= len;
        direction.y /= len;
        direction.z /= len;
    }

    Vector3 right = {
        -std::sin(yawRad),
        0.0f,
        std::cos(yawRad)
    };

    Vector3 up = {0.0f, 1.0f, 0.0f};

    Vector3 pos = getPosition();

    if (app->isKeyPressed(GLFW_KEY_W)) {
        pos = pos + direction * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_S)) {
        pos = pos - direction * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_A)) {
        pos = pos - right * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_D)) {
        pos = pos + right * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_E) || app->isKeyPressed(GLFW_KEY_SPACE)) {
        pos = pos + up * speed;
    }
    if (app->isKeyPressed(GLFW_KEY_Q) || app->isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        pos = pos - up * speed;
    }

    setPosition(pos);
}

void OrbitCamera::update(Application* app, float) {
    double scrollY = app->getScrollY();
    if (scrollY != 0.0) {
        distance -= static_cast<float>(scrollY) * 0.5f;
        if (distance < 1.0f) distance = 1.0f;
        if (distance > 100.0f) distance = 100.0f;
    }

    double mouseX = app->getCursorX();
    double mouseY = app->getCursorY();

    if (app->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        if (!isDragging) {
            isDragging = true;
            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }

        double dx = mouseX - lastMouseX;
        double dy = mouseY - lastMouseY;

        Vector3 rot = getRotation();
        rot.y += static_cast<float>(dx) * 0.25f;
        rot.x += static_cast<float>(dy) * 0.25f;

        if (rot.x > 89.0f) rot.x = 89.0f;
        if (rot.x < -89.0f) rot.x = -89.0f;

        setRotation(rot);

        lastMouseX = mouseX;
        lastMouseY = mouseY;
    } else {
        isDragging = false;
    }

    if (app->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (!isPanning) {
            isPanning = true;
            lastPanX = mouseX;
            lastPanY = mouseY;
        }

        double dx = mouseX - lastPanX;
        double dy = mouseY - lastPanY;

        float yawRad = transform.rotationDeg.y * std::numbers::pi_v<float> / 180.0f;

        Vector3 right = { -std::sin(yawRad), 0.0f, std::cos(yawRad) };
        Vector3 up = { 0.0f, 1.0f, 0.0f };

        float panSpeed = distance * 0.002f;
        
        target = target - right * (static_cast<float>(dx) * panSpeed) + up * (static_cast<float>(dy) * panSpeed);

        lastPanX = mouseX;
        lastPanY = mouseY;
    } else {
        isPanning = false;
    }
}
