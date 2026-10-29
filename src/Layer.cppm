export module shark:Layer;

import std;

export namespace shark {
class Application;
class Renderer;

class Layer {
public:
    virtual ~Layer() = default;

    Layer(const Layer&)            = delete ("layers are owned by Application");
    Layer& operator=(const Layer&) = delete ("layers are owned by Application");
    Layer(Layer&&)                 = delete ("Application hands out raw Layer pointers");
    Layer& operator=(Layer&&)      = delete ("Application hands out raw Layer pointers");

    inline virtual void onAttach() {}
    inline virtual void onDetach() {}

    inline virtual void onFixedUpdate(float) {}
    inline virtual void onUpdate(float) {}
    inline virtual void onRender(Renderer&) {}
    inline void         setApp(Application* application) noexcept { app = application; }

protected:
    Layer() = default;

    [[nodiscard]] inline Application* getApp() const noexcept { return app; }

private:
    Application* app = nullptr;

    friend class Application;
};

struct Unique {};

template <typename T>
concept UniqueLayer = std::is_base_of_v<Unique, T>;

} // namespace shark
