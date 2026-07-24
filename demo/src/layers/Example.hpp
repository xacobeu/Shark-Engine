#pragma once

#include <SharkEngine/Layer.hpp>
#include <SharkEngine/renderer/Renderer.hpp>

class Example : public Layer {
public:
    void onAttach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

private:
    Renderer::GameObject cube;
    bool created = false;
    float elapsed = 0.0f;
    int frameCount = 0;
};
