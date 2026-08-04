module;
#include <functional>
#ifdef SHARK_NO_GLAD
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

export module SharkEngine:Window;
import :RenderDeviceAbstractions;

export class Window {
public:
    Window(int width, int height, const char* title, GraphicsAPI api) : usesOpenGL(api == GraphicsAPI::OpenGL) {
        if (!glfwInit()) {
            printf("GLFW Init Failed!\n");
            return;
        }

        if (usesOpenGL) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        } else {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!window) {
            printf("GLFW Window Creation Failed!\n");
            glfwTerminate();
            return;
        }

        if (usesOpenGL) glfwMakeContextCurrent(window);

#ifndef __EMSCRIPTEN__
        if (usesOpenGL && !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            printf("Failed to initialize GLAD\n");
            return;
        }
#endif

        glfwSetWindowUserPointer(window, this);

        glfwSetKeyCallback(window, _keyCallback);
        glfwSetFramebufferSizeCallback(window, _resizeCallback);
        glfwSetMouseButtonCallback(window, _mouseButtonCallback);
        glfwSetCursorPosCallback(window, _cursorPosCallback);
        glfwSetScrollCallback(window, _scrollCallback);
        glfwSetCharCallback(window, _charCallback);

        initImGui();
    }

    ~Window() {
        if (usesOpenGL) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
        }
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void initImGui() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.Fonts->Clear();
        io.Fonts->AddFontFromFileTTF("res/font/AtkinsonHyperlegible-Regular.ttf", 20.0f);

        ImGui::StyleColorsDark();

        if (!usesOpenGL) return;

        ImGui_ImplGlfw_InitForOpenGL(window, false);
#ifdef __EMSCRIPTEN__
        ImGui_ImplOpenGL3_Init("#version 300 es");
#else
        ImGui_ImplOpenGL3_Init("#version 450");
#endif
    }

    void beginImGuiFrame() {
        if (!usesOpenGL) {
            ImGui::NewFrame();
            return;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void endImGuiFrame() {
        ImGui::Render();
        if (usesOpenGL) {
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }

    struct Events {
        std::function<void(int button, int action, int mods)> onMouseClick;
        std::function<void(double x, double y)>               onMouseMove;
        std::function<void(double x, double y)>               onScroll;
        std::function<void(int key, int action, int mods)>    onKey;
        std::function<void(int width, int height)>            onResize;
    } events;

    [[nodiscard]] bool shouldClose() const { return glfwWindowShouldClose(window); }
    void close() { glfwSetWindowShouldClose(window, GLFW_TRUE); }

    void swapBuffers() const {
        if (usesOpenGL) {
            glfwSwapBuffers(window);
        }
    }

    void pollEvents() const {
#ifdef __EMSCRIPTEN__
        int width = EM_ASM_INT({
            var canvas = document.getElementById('canvas');
            return canvas ? canvas.clientWidth : window.innerWidth;
        });
        int height = EM_ASM_INT({
            var canvas = document.getElementById('canvas');
            return canvas ? canvas.clientHeight : window.innerHeight;
        });

        int currentWidth, currentHeight;
        glfwGetWindowSize(window, &currentWidth, &currentHeight);
        if (currentWidth != width || currentHeight != height) {
            glfwSetWindowSize(window, width, height);
        }
#endif
        glfwPollEvents();
    }

    void getDimensions(int& w, int& h) const {
        glfwGetFramebufferSize(window, &w, &h);
    }

    [[nodiscard]] bool isKeyPressed(int key) const {
        return glfwGetKey(window, key) == GLFW_PRESS;
    }

    [[nodiscard]] bool isMouseButtonPressed(int button) const {
        return glfwGetMouseButton(window, button) == GLFW_PRESS;
    }

    [[nodiscard]] double getCursorX() const {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return x;
    }

    [[nodiscard]] double getCursorY() const {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return y;
    }

    [[nodiscard]] double getScrollX() const { return scrollX; }
    [[nodiscard]] double getScrollY() const { return scrollY; }
    void resetScroll() { scrollX = 0.0; scrollY = 0.0; }

    [[nodiscard]] int getWidth() const {
        int w, h;
        getDimensions(w, h);
        return w;
    }

    [[nodiscard]] int getHeight() const {
        int w, h;
        getDimensions(w, h);
        return h;
    }

    void setVSync(bool enabled) {
#ifndef __EMSCRIPTEN__
        glfwSwapInterval(enabled ? 1 : 0);
#else
        (void)enabled;
#endif
    }

private:
    GLFWwindow* window = nullptr;
    bool usesOpenGL = true;
    double scrollX = 0.0;
    double scrollY = 0.0;

    static void _mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
        Window* windowClass = static_cast<Window*>(glfwGetWindowUserPointer(win));
        if (windowClass->events.onMouseClick) {
            windowClass->events.onMouseClick(button, action, mods);
        }
        ImGui_ImplGlfw_MouseButtonCallback(win, button, action, mods);
    }

    static void _cursorPosCallback(GLFWwindow* win, double x, double y) {
        Window* windowClass = static_cast<Window*>(glfwGetWindowUserPointer(win));
        if (windowClass->events.onMouseMove) {
            windowClass->events.onMouseMove(x, y);
        }
        ImGui_ImplGlfw_CursorPosCallback(win, x, y);
    }

    static void _scrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
        Window* windowClass = static_cast<Window*>(glfwGetWindowUserPointer(win));
        windowClass->scrollX += xoffset;
        windowClass->scrollY += yoffset;
        if (windowClass->events.onScroll) {
            windowClass->events.onScroll(xoffset, yoffset);
        }
        ImGui_ImplGlfw_ScrollCallback(win, xoffset, yoffset);
    }

    static void _keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
        Window* windowClass = static_cast<Window*>(glfwGetWindowUserPointer(win));
        if (windowClass->events.onKey) {
            windowClass->events.onKey(key, action, mods);
        }
        ImGui_ImplGlfw_KeyCallback(win, key, scancode, action, mods);
    }

    static void _charCallback(GLFWwindow* win, unsigned int codepoint) {
        ImGui_ImplGlfw_CharCallback(win, codepoint);
    }

    static void _resizeCallback(GLFWwindow* win, int width, int height) {
        Window* windowClass = static_cast<Window*>(glfwGetWindowUserPointer(win));
        if (windowClass->events.onResize) {
            windowClass->events.onResize(width, height);
        }
    }
};
