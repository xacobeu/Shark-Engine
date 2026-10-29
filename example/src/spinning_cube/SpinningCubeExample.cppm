export module examples:spinning_cube.SpinningCubeExample;

import shark;

using namespace shark;

export {
class SpinningCubeExample : public Layer {
public:
    void onAttach() override;
    void onDetach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

private:
    Scene        scene;
    Entity*      cube       = nullptr;
    Light*       orbitLight = nullptr;
    OrbitCamera* camera     = nullptr;
    float        elapsed    = 0.0f;
    int          frameCount = 0;
};

} // export
