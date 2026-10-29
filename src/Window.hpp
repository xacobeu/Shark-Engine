#pragma once

#include <functional>

struct GLFWwindow;

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    Window(const Window&)            = delete ("owns the GLFW window; move it instead");
    Window& operator=(const Window&) = delete ("owns the GLFW window; move it instead");

    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    void initImGui();

    static void beginImGuiFrame();
    static void endImGuiFrame();

    struct Events {
        std::function<void(int button, int action, int mods)> onMouseClick;
        std::function<void(double x, double y)>               onMouseMove;
        std::function<void(double x, double y)>               onScroll;
        std::function<void(int key, int action, int mods)>    onKey;
        std::function<void(int width, int height)>            onResize;
    };

    [[nodiscard]] Events&       getEvents() noexcept { return events; }
    [[nodiscard]] const Events& getEvents() const noexcept { return events; }

    [[nodiscard]] bool shouldClose() const;
    void               close();

    void pollEvents();

    void swapBuffers() const;
    void getDimensions(int& width, int& height) const;

    [[nodiscard]] bool isKeyPressed(int key) const;
    [[nodiscard]] bool isMouseButtonPressed(int button) const;

    [[nodiscard]] double getCursorX() const;
    [[nodiscard]] double getCursorY() const;

    [[nodiscard]] double getScrollX() const { return scrollX; }
    [[nodiscard]] double getScrollY() const { return scrollY; }
    void                 resetScroll() {
        scrollX = 0.0;
        scrollY = 0.0;
    }

    [[nodiscard]] int getWidth() const;
    [[nodiscard]] int getHeight() const;

    static void setVSync(bool enabled);

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
