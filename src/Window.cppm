export module shark:Window;

import std;

import :Input;

extern "C++" struct GLFWwindow;

export namespace shark {
class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    Window(const Window&)            = delete ("owns the GLFW window");
    Window& operator=(const Window&) = delete ("owns the GLFW window");

    void initImGui();

    static void beginImGuiFrame();
    static void endImGuiFrame();

    [[nodiscard]] static bool isMouseCaptured() noexcept;
    [[nodiscard]] static bool isKeyboardCaptured() noexcept;

    struct Events {
        std::function<void(int button, int action, int mods)> onMouseClick;
        std::function<void(double x, double y)>               onMouseMove;
        std::function<void(double x, double y)>               onScroll;
        std::function<void(int key, int action, int mods)>    onKey;
        std::function<void(int width, int height)>            onResize;
    };

    [[nodiscard]] inline Events&       getEvents() noexcept { return events; }
    [[nodiscard]] inline const Events& getEvents() const noexcept { return events; }

    [[nodiscard]] bool shouldClose() const;
    void               close();

    void pollEvents();

    void swapBuffers() const;
    void getDimensions(int& width, int& height) const;

    [[nodiscard]] bool        isKeyPressed(int key) const;
    [[nodiscard]] inline bool isKeyPressed(Key key) const { return isKeyPressed(static_cast<int>(key)); }
    [[nodiscard]] bool        isMouseButtonPressed(int button) const;
    [[nodiscard]] inline bool isMouseButtonPressed(MouseButton button) const {
        return isMouseButtonPressed(static_cast<int>(button));
    }

    [[nodiscard]] double getCursorX() const;
    [[nodiscard]] double getCursorY() const;

    [[nodiscard]] inline double getScrollX() const { return scrollX; }
    [[nodiscard]] inline double getScrollY() const { return scrollY; }
    inline void                 resetScroll() {
        scrollX = 0.0;
        scrollY = 0.0;
    }

    [[nodiscard]] int getWidth() const;
    [[nodiscard]] int getHeight() const;

private:
    GLFWwindow* window  = nullptr;
    double      scrollX = 0.0;
    double      scrollY = 0.0;

    Events events{};

    static void _mouseButtonCallback(GLFWwindow* win, int button, int action, int mods);
    static void _cursorPosCallback(GLFWwindow* win, double x, double y);
    static void _scrollCallback(GLFWwindow* win, double xoffset, double yoffset);
    static void _keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods);
    static void _charCallback(GLFWwindow* w, unsigned int codepoint);
    static void _resizeCallback(GLFWwindow* win, int width, int height);
};

} // namespace shark
