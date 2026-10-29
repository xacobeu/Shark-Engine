#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "AdaptiveFMM.hpp"
#include "Particle.hpp"

class FMMLayer : public Layer {
public:
    static constexpr std::size_t DEFAULT_NUM_PARTICLES = 10000;
    static constexpr Vector3     BOUNDS                = {200.0f, 200.0f, 200.0f};
    static constexpr float       PARTICLE_RADIUS       = 0.08f;
    static constexpr float       G_CONSTANT            = 2.5f;

    void onAttach() override;
    void onDetach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

    void resetSimulation(std::size_t count);

private:
    enum class Distribution : std::uint8_t { Disc, Uniform };

    struct KuzminDiskConfig {
        float         particleMass       = 0.05f;
        float         coreRadius         = 14.0f;
        float         maxRadiusRatio     = 0.35f;
        float         verticalDispersion = 1.5f;
        float         thermalDispersion  = 0.06f;
        std::uint32_t seed               = 42;
    };

    struct UniformConfig {
        float         particleMass   = 0.05f;
        float         maxRadiusRatio = 0.35f;
        std::uint32_t seed           = 42;
    };

    static void generateKuzminDisk(
        std::span<fmm::Particle> particles, std::span<InstanceData> instanceData, const KuzminDiskConfig& cfg
    );
    static void generateUniformDistribution(
        std::span<fmm::Particle> particles, std::span<InstanceData> instanceData, const UniformConfig& cfg
    );

    void                renderImGui();
    void                solve();
    [[nodiscard]] float measureColorMaxSpeed() const;

    static constexpr std::size_t ERROR_UPDATE_INTERVAL_FRAMES = 20;
    static constexpr std::size_t ERROR_SAMPLE_COUNT           = 100;

    // Top of the colormap sits at this multiple of the RMS particle speed.
    static constexpr float COLOR_RMS_MULTIPLIER          = 1.6f;
    static constexpr float COLOR_RANGE_SMOOTHING_SECONDS = 0.5f;

    Scene            scene;
    InstancedEntity* particleGroup = nullptr;

    std::vector<fmm::Particle> particles;

    AdaptiveFMM         fmm;
    AdaptiveFMM::Config fmmConfig;
    float               lastFmmTimeMs    = 0.0f;
    float               observedErrorPct = 0.0f;
    std::size_t         frameCount       = 0;
    float               colorMaxSpeed    = 1.0f;

    Distribution distribution         = Distribution::Disc;
    bool         isPaused             = false;
    int          uiParticleCount      = DEFAULT_NUM_PARTICLES;
    std::size_t  currentParticleCount = DEFAULT_NUM_PARTICLES;
};
