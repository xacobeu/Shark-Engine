export module examples.spinning_cube;

import std;

import examples.base;
import shark;

export {
class SpinningCubeExample : public examples::Example {
public:
    void onAttach() override;
    void onDetach() override;
    void onUpdate(float deltaTime) override;
    void onRender(shark::Renderer& renderer) override;

    [[nodiscard]] bool hasControls() const override { return false; }

private:
    shark::Scene        scene;
    shark::Entity*      cube       = nullptr;
    shark::Light*       orbitLight = nullptr;
    shark::OrbitCamera* camera     = nullptr;

    float elapsed    = 0.0f;
    int   frameCount = 0;
};
} // export

void SpinningCubeExample::onAttach() {
    getApp()->setScene(&scene);

    camera = scene.add<shark::OrbitCamera>()->setDistance(6.0f);

    cube = scene.add<shark::Entity>()
               ->setMesh(std::make_shared<shark::Mesh>(shark::MeshFactory::cube()))
               ->setScale({.x = 1.0f, .y = 1.0f, .z = 1.0f})
               ->setPosition({.x = 0.0f, .y = 0.0f, .z = 0.0f})
               ->setTexture(getApp()->getDevice()->createTexture("res/textures/scifi_crate.png"));

    scene.add<shark::Light>()
        ->setType(shark::LightType::Ambient)
        ->setColor({0.6f, 0.7f, 1.0f, 1.0f})
        ->setIntensity(0.15f);
    scene.add<shark::Light>()
        ->setType(shark::LightType::Directional)
        ->setDirection({-0.3f, -1.0f, -0.5f})
        ->setColor({0.8f, 0.85f, 1.0f, 1.0f})
        ->setIntensity(0.35f);
    orbitLight = scene.add<shark::Light>()
                     ->setType(shark::LightType::Point)
                     ->setColor({1.0f, 0.75f, 0.45f, 1.0f})
                     ->setIntensity(4.0f)
                     ->setRange(8.0f);
}

void SpinningCubeExample::onDetach() {
    if ((getApp() != nullptr) && getApp()->getScene() == &scene) {
        getApp()->setScene(nullptr);
    }
    cube       = nullptr;
    orbitLight = nullptr;
    camera     = nullptr;
    scene.clear();
}

void SpinningCubeExample::onUpdate(float deltaTime) {
    elapsed += deltaTime;
    ++frameCount;

    cube->setRotation({.x = 0.0f, .y = cube->getRotation().y + (deltaTime * 35.0f), .z = 0.0f});

    constexpr float ORBIT_RADIUS = 2.2f;
    constexpr float ORBIT_SPEED  = 2.0f;
    orbitLight->setPosition({
        .x = std::cos(elapsed * ORBIT_SPEED) * ORBIT_RADIUS,
        .y = 1.0f + (std::sin(elapsed * ORBIT_SPEED * 1.7f) * 0.6f),
        .z = std::sin(elapsed * ORBIT_SPEED) * ORBIT_RADIUS,
    });
}

void SpinningCubeExample::onRender(shark::Renderer& renderer) {
    renderer.render(scene, getApp()->getWidth(), getApp()->getHeight());
}
