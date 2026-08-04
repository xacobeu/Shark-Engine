module;
#include <type_traits>

export module SharkEngine:Layer;

export class Application;
export class Renderer;

export class Layer {
public:
    virtual ~Layer() = default;

    virtual void onAttach() {}
    virtual void onDetach() {}

    virtual void onUpdate(float) {}
    virtual void onRender(Renderer&) {}

protected:
    Application* app = nullptr;

    friend class Application;
};

export struct Unique {};

export template<typename T>
concept UniqueLayer = std::is_base_of_v<Unique, T>;
