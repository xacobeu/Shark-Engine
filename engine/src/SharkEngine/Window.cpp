#include <SharkEngine/Window.hpp>

#ifdef SHARK_NO_GLAD
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <cstdio>
#include <filesystem>

Window::Window(int width, int height, const char *title) {

    if (!glfwInit()) {
        printf("GLFW Init Failed!\n");
        return;
    }

#ifdef __EMSCRIPTEN__
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        printf("GLFW Window Creation Failed!\n");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);

#ifndef __EMSCRIPTEN__
    // Load OpenGL.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
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

Window::~Window() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.Fonts->Clear();
    if (std::filesystem::exists("res/font/AtkinsonHyperlegible-Regular.ttf")) {
        io.Fonts->AddFontFromFileTTF("res/font/AtkinsonHyperlegible-Regular.ttf", 20.0f);
    } else {
        io.Fonts->AddFontDefault();
    }

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, false);
#ifdef __EMSCRIPTEN__
    ImGui_ImplOpenGL3_Init("#version 300 es");
#else
    ImGui_ImplOpenGL3_Init("#version 450");
#endif
}

void Window::beginImGuiFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Window::endImGuiFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

// Mouse callbacks

void Window::_mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(win, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    Window *window = static_cast<Window *>(glfwGetWindowUserPointer(win));
    if (window && window->events.onMouseClick)
        window->events.onMouseClick(button, action, mods);
}

void Window::_cursorPosCallback(GLFWwindow *win, double x, double y) {
    ImGui_ImplGlfw_CursorPosCallback(win, x, y);
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    Window *window = static_cast<Window *>(glfwGetWindowUserPointer(win));
    if (window && window->events.onMouseMove)
        window->events.onMouseMove(x, y);
}

void Window::_scrollCallback(GLFWwindow *win, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(win, xoffset, yoffset);

    // Debug: Check if ImGui is consuming the scroll
    if (ImGui::GetIO().WantCaptureMouse) {
        // printf("ImGui captured scroll\n");
        return;
    }

    Window *window = static_cast<Window *>(glfwGetWindowUserPointer(win));
    if (window) {
        window->scrollX += xoffset;
        window->scrollY += yoffset;
        if (window->events.onScroll)
            window->events.onScroll(xoffset, yoffset);
    }
}

// Keyboard callbacks

void Window::_keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(win, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard)
        return;

    Window *window = static_cast<Window *>(glfwGetWindowUserPointer(win));
    if (window && window->events.onKey)
        window->events.onKey(key, action, mods);
}

void Window::_charCallback(GLFWwindow *w, unsigned int codepoint) {
    ImGui_ImplGlfw_CharCallback(w, codepoint);
}

// Window callbacks

void Window::_resizeCallback(GLFWwindow *win, int width, int height) {
    Window *window = static_cast<Window *>(glfwGetWindowUserPointer(win));
    if (window && window->events.onResize)
        window->events.onResize(width, height);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::close() {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Window::swapBuffers() const {
    glfwSwapBuffers(window);
}

void Window::pollEvents() const {
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

bool Window::isKeyPressed(int key) const {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

double Window::getCursorX() const {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return x;
}

double Window::getCursorY() const {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return y;
}

void Window::getDimensions(int &width, int &height) const {
    glfwGetFramebufferSize(window, &width, &height);
}

int Window::getWidth() const {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    return w;
}

int Window::getHeight() const {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    return h;
}

void Window::setVSync(bool enabled) {
#ifndef __EMSCRIPTEN__
    glfwSwapInterval(enabled ? 1 : 0);
#else
    (void)enabled;
#endif
}
