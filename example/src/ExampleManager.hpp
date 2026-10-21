#pragma once

#include <memory>
#include <string_view>

#include "Layer.hpp"

enum class ExampleType : std::uint8_t { FluidSim = 0, MolecularDynamics, SpinningCube, RayTracing, Count };

class ExampleManager : public Layer {
public:
    ExampleManager();
    ~ExampleManager() override;

    void onAttach() override;
    void onDetach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

    void switchExample(ExampleType type);
    void reloadCurrent();

    [[nodiscard]] ExampleType             getCurrentType() const noexcept { return currentType; }
    [[nodiscard]] static std::string_view getExampleName(ExampleType type) noexcept;

private:
    void                   renderTopBar();
    void                   processHotkeys();
    void                   applyPendingSwitch();
    std::unique_ptr<Layer> createExample(ExampleType type);

    ExampleType currentType = ExampleType::FluidSim;
    ExampleType pendingType = ExampleType::FluidSim;
    bool        needsReload = false;

    std::unique_ptr<Layer> activeExample;
};
