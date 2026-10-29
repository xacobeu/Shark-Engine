#pragma once

#include <vector>

#include "Particle.hpp"
#include "SharkEngine.hpp"

class FluidSim : public Layer {
public:
    static constexpr float   PARTICLE_RADIUS = 0.05f;
    static constexpr float   GRAVITY         = 9.8f;
    static constexpr Vector3 BOUNDS          = {4.0f, 4.0f, 4.0f};
    static constexpr float   DAMPING         = 0.9f;

    enum class SpawnPreset : std::uint8_t {
        Waves,
        CenteredBlock,
    };

    void onAttach() override;
    void onDetach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

    void resetSimulation();
    void resetBox();

private:
    SpawnPreset currentPreset = SpawnPreset::Waves;
    void        renderImGui();

    Scene            scene;
    OrbitCamera*     camera         = nullptr;
    InstancedEntity* particleGroup  = nullptr;
    Entity*          boxFrameEntity = nullptr;

    // Box tilt dynamics and force state
    Vector3 boxRot{0.0f, 0.0f, 0.0f};    // in degrees (pitch X, yaw Y, roll Z)
    Vector3 boxAngVel{0.0f, 0.0f, 0.0f}; // in deg/s
    Vector3 boxAngAcc{0.0f, 0.0f, 0.0f};
    Vector3 prevBoxAngVel{0.0f, 0.0f, 0.0f};
    Vector3 boxTargetRot{0.0f, 0.0f, 0.0f};

    bool   isDraggingBox   = false;
    double lastMouseX      = 0.0;
    double lastMouseY      = 0.0;
    float  tiltSensitivity = 0.25f;

    int   currentParticleCount = 10000;
    int   uiParticleCount      = 10000;
    bool  isPaused             = false;
    float gravity              = 9.8f;
    float restitution          = 0.25f;
    float viscosity            = 0.018f;
    float relaxation           = 0.40f;
    float friction             = 0.0f;

    std::vector<Particle> particles;
    SpatialHashGrid       grid;
};
