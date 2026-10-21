#pragma once
#include <type_traits>

class Application;
class Renderer;

class Layer {
public:
    virtual ~Layer() = default;

    Layer(const Layer&)            = delete;
    Layer& operator=(const Layer&) = delete;
    Layer(Layer&&)                 = delete;
    Layer& operator=(Layer&&)      = delete;

    virtual void onAttach() {}
    virtual void onDetach() {}

    virtual void onUpdate(float) {}
    virtual void onRender(Renderer&) {}
    void         setApp(Application* application) noexcept { app = application; }

protected:
    Layer() = default;

    [[nodiscard]] Application* getApp() const noexcept { return app; }

private:
    Application* app = nullptr;

    friend class Application;
};

struct Unique {};

template <typename T>
concept UniqueLayer = std::is_base_of_v<Unique, T>;
