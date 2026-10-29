#pragma once

#include <memory>
#include <string_view>

#include "Layer.hpp"

enum class ExampleType : std::uint8_t {
    FluidSim = 0,
    FastMultipoleMethod,
    MolecularDynamics,
    SpinningCube,
    RayTracing,
    Count,
};

class ExampleManager : public Layer {
public:
    ExampleManager();
    ~ExampleManager() override;

    ExampleManager(const ExampleManager&)            = delete ("layers are owned by Application");
    ExampleManager& operator=(const ExampleManager&) = delete ("layers are owned by Application");
    ExampleManager(ExampleManager&&)                 = delete ("Application hands out raw Layer pointers");
    ExampleManager& operator=(ExampleManager&&)      = delete ("Application hands out raw Layer pointers");

    void onAttach() override;
    void onDetach() override;
    void onFixedUpdate(float fixedDt) override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

    void switchExample(ExampleType type);
    void reloadCurrent();

    [[nodiscard]] ExampleType             getCurrentType() const noexcept { return currentType; }
    [[nodiscard]] static std::string_view getExampleName(ExampleType type) noexcept;

private:
    void                          renderTopBar();
    void                          processHotkeys();
    void                          applyPendingSwitch();
    static std::unique_ptr<Layer> createExample(ExampleType type);

    ExampleType currentType = ExampleType::FastMultipoleMethod;
    ExampleType pendingType = ExampleType::FastMultipoleMethod;
    bool        needsReload = false;

    std::unique_ptr<Layer> activeExample;
};
