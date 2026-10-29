export module shark:Application;

import std;

import :Input;
import :Window;
import :renderer.Renderer;
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

} // namespace shark
