#pragma once

#include <SharkEngine/Layer.hpp>
#include <SharkEngine/renderer/Renderer.hpp>
#include <SharkEngine/renderer/scene/Scene.hpp>

class BoxExample : public Layer {
public:
    void onAttach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

private:
    Scene scene;
    FlyingCamera* camera = nullptr;
    Entity* tallBox;

    static constexpr int DT_FILTER_SIZE = 10;
    float dtHistory[DT_FILTER_SIZE] = {};
    int dtHistoryCount = 0;
    int dtHistoryIndex = 0;

    float fpsTimer = 0.0f;
    float displayedFps = 0.0f;
};
