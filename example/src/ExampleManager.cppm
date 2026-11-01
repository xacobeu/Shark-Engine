module;

#include "imgui.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

export module examples.manager;

import std;

import examples.fast_multipole_method;
import examples.fluid_simulation;
import examples.molecular_dynamics;
import examples.ray_tracing;
import examples.spinning_cube;
import examples.base;

import shark;

using namespace shark;

export enum class ExampleType : std::uint8_t {
    FluidSimulation = 0,
    FastMultipoleMethod,
    MolecularDynamics,
    SpinningCube,
    RayTracing,
    Count,
};

void SetupForestGreenStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Sizing and Spacing.
    style.WindowPadding     = ImVec2(10.0f, 10.0f);
    style.FramePadding      = ImVec2(6.0f, 4.0f);
    style.CellPadding       = ImVec2(6.0f, 4.0f);
    style.ItemSpacing       = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing  = ImVec2(6.0f, 4.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing     = 20.0f;
    style.ScrollbarSize     = 14.0f;
    style.GrabMinSize       = 12.0f;

    // Borders.
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize  = 1.0f;
    style.PopupBorderSize  = 1.0f;
    style.FrameBorderSize  = 1.0f;
    style.TabBorderSize    = 0.0f;

    // Rounding.
    style.WindowRounding    = 6.0f;
    style.ChildRounding     = 4.0f;
    style.FrameRounding     = 4.0f;
    style.PopupRounding     = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding      = 4.0f;
    style.LogSliderDeadzone = 4.0f;
    style.TabRounding       = 4.0f;
}

export class ExampleManager : public Layer {
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

    [[nodiscard]] inline ExampleType      getCurrentType() const noexcept { return currentType; }
    [[nodiscard]] static std::string_view getExampleName(ExampleType type) noexcept;

private:
    void                          renderTopBar();
    void                          processHotkeys();
    void                          applyPendingSwitch();
    static std::unique_ptr<examples::Example> createExample(ExampleType type);

    ExampleType currentType = ExampleType::FluidSimulation;
    ExampleType pendingType = ExampleType::FluidSimulation;
    bool        needsReload = false;

    std::unique_ptr<examples::Example> activeExample;
};

ExampleManager::ExampleManager() {
    SetupForestGreenStyle();

#ifdef __EMSCRIPTEN__
    const bool touchScreen = EM_ASM_INT({ return matchMedia('(pointer: coarse)').matches ? 1 : 0; }) != 0;
    if (touchScreen) {
        ImGuiStyle& style       = ImGui::GetStyle();
        style.TouchExtraPadding = ImVec2(4.0f, 4.0f);
        style.GrabMinSize       = 20.0f;
        style.ScrollbarSize     = 20.0f;
    }
#endif
}

ExampleManager::~ExampleManager() {
    onDetach();
}

std::string_view ExampleManager::getExampleName(ExampleType type) noexcept {
    switch (type) {
    case ExampleType::FluidSimulation:
        return "Fluid Simulation";
    case ExampleType::FastMultipoleMethod:
        return "Fast Multipole Method";
    case ExampleType::MolecularDynamics:
        return "Molecular Dynamics";
    case ExampleType::SpinningCube:
        return "Spinning Cube";
    case ExampleType::RayTracing:
        return "GPU Raytracer";
    default:
        return "Unknown";
    }
}

std::unique_ptr<examples::Example> ExampleManager::createExample(ExampleType type) {
    switch (type) {
    case ExampleType::FluidSimulation:
        return std::make_unique<FluidSimulationExample>();
    case ExampleType::FastMultipoleMethod:
        return std::make_unique<FastMultipoleMethodExample>();
    case ExampleType::MolecularDynamics:
        return std::make_unique<MolecularDynamicsExample>();
    case ExampleType::SpinningCube:
        return std::make_unique<SpinningCubeExample>();
    case ExampleType::RayTracing:
        return std::make_unique<RayTracingExample>();
    default:
        return nullptr;
    }
}

void ExampleManager::switchExample(ExampleType type) {
    if (type >= ExampleType::FluidSimulation && type < ExampleType::Count) {
        pendingType = type;
    }
}

void ExampleManager::reloadCurrent() {
    needsReload = true;
}

void ExampleManager::onAttach() {
    std::println("[ExampleManager] Attached. Initializing default example: {}", getExampleName(currentType));
    applyPendingSwitch();
}

void ExampleManager::onDetach() {
    if (activeExample) {
        std::println("[ExampleManager] Detaching active example: {}", getExampleName(currentType));
        activeExample->onDetach();
        if (getApp() != nullptr) {
            getApp()->setScene(nullptr);
            getApp()->getRenderer().clearCache();
        }
        activeExample.reset();
    }
}

void ExampleManager::applyPendingSwitch() {
    if (pendingType != currentType || needsReload || !activeExample) {
        if (activeExample) {
            std::println(
                "[ExampleManager] Transitioning from {} to {}", getExampleName(currentType), getExampleName(pendingType)
            );
            activeExample->onDetach();
            if (getApp() != nullptr) {
                getApp()->setScene(nullptr);
                getApp()->getRenderer().clearCache();
            }
            activeExample.reset();
        }

        currentType = pendingType;
        needsReload = false;

        activeExample = createExample(currentType);
        if (activeExample && (getApp() != nullptr)) {
            activeExample->setApp(getApp());
            activeExample->onAttach();
        }
    }
}

void ExampleManager::processHotkeys() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_1, false) || ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
        switchExample(ExampleType::FluidSimulation);
    } else if (ImGui::IsKeyPressed(ImGuiKey_2, false) || ImGui::IsKeyPressed(ImGuiKey_F2, false)) {
        switchExample(ExampleType::FastMultipoleMethod);
    } else if (ImGui::IsKeyPressed(ImGuiKey_3, false) || ImGui::IsKeyPressed(ImGuiKey_F3, false)) {
        switchExample(ExampleType::MolecularDynamics);
    } else if (ImGui::IsKeyPressed(ImGuiKey_4, false) || ImGui::IsKeyPressed(ImGuiKey_F4, false)) {
        switchExample(ExampleType::SpinningCube);
    } else if (ImGui::IsKeyPressed(ImGuiKey_5, false) || ImGui::IsKeyPressed(ImGuiKey_F5, false)) {
        switchExample(ExampleType::RayTracing);
    } else if (ImGui::IsKeyPressed(ImGuiKey_R, false)) {
        reloadCurrent();
    }
}

void ExampleManager::onFixedUpdate(float fixedDt) {
    if (activeExample) {
        activeExample->onFixedUpdate(fixedDt);
    }
}

void ExampleManager::onUpdate(float deltaTime) {
    processHotkeys();
    applyPendingSwitch();

    if (activeExample) {
        activeExample->onUpdate(deltaTime);
    }
}

void ExampleManager::onRender(Renderer& renderer) {
    if (activeExample) {
        activeExample->onRender(renderer);
    }

    renderTopBar();
}

void ExampleManager::renderTopBar() {
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }

    static constexpr std::array<std::pair<ExampleType, const char*>, 5> quickButtons{{
        {ExampleType::FluidSimulation, "Fluid"},
        {ExampleType::FastMultipoleMethod, "FMM"},
        {ExampleType::MolecularDynamics, "Molecular"},
        {ExampleType::SpinningCube, "Cube"},
        {ExampleType::RayTracing, "Raytracer"},
    }};
    static constexpr const char* reloadLabel   = "Reload (R)";
    static constexpr const char* controlsLabel = "Controls";

    const ImGuiStyle& style       = ImGui::GetStyle();
    auto              buttonWidth = [&style](const char* label) {
        return ImGui::CalcTextSize(label).x + (2.0f * style.FramePadding.x) + style.ItemSpacing.x;
    };

    ImGuiIO&          io    = ImGui::GetIO();
    const float       fps   = io.Framerate;
    const float       ms    = (fps > 0.0f) ? (1000.0f / fps) : 0.0f;
    const std::string full  = std::format("{} | {:.1f} FPS ({:.2f} ms)", getExampleName(currentType), fps, ms);
    const std::string brief = std::format("{:.0f} FPS", fps);

    float fullWidth = ImGui::GetCursorPosX() + buttonWidth(reloadLabel) + buttonWidth("Controls     ") +
                      ImGui::CalcTextSize(full.c_str()).x + 18.0f;
    for (const auto& [type, label] : quickButtons) {
        fullWidth += buttonWidth(label);
    }
    const bool compact = fullWidth > ImGui::GetWindowWidth();

    auto pushButtonColors = [](bool active) {
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.45f, 0.85f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.55f, 0.95f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.20f, 0.50f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.35f, 0.70f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.80f, 1.0f));
        }
    };

    auto beginDropdown = [&style](std::string_view label, std::string_view id) {
        ImDrawList* barDrawList = ImGui::GetWindowDrawList();
        const bool  open        = ImGui::BeginMenu(std::format("{}     ###{}", label, id).c_str());

        const ImVec2 itemMin = ImGui::GetItemRectMin();
        const ImVec2 itemMax = ImGui::GetItemRectMax();
        const float  half    = ImGui::GetFontSize() * 0.22f;
        const ImVec2 tip(itemMax.x - (style.ItemSpacing.x * 0.5f) - half - 4.0f, (itemMin.y + itemMax.y) * 0.5f);
        const ImVec2 left(tip.x - half, tip.y - (half * 0.5f));
        const ImVec2 right(tip.x + half, tip.y - (half * 0.5f));
        const ImVec2 bottom(tip.x, tip.y + (half * 0.6f));
        const ImU32  color = ImGui::GetColorU32(ImGuiCol_Text);
        barDrawList->AddLine(left, bottom, color, 2.0f);
        barDrawList->AddLine(bottom, right, color, 2.0f);
        return open;
    };

    if (compact) {
        if (beginDropdown(getExampleName(currentType), "examples")) {
            for (const auto& [type, label] : quickButtons) {
                if (ImGui::MenuItem(getExampleName(type).data(), nullptr, currentType == type)) {
                    switchExample(type);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Reload", "R")) {
                reloadCurrent();
            }
            ImGui::EndMenu();
        }
    } else {
        // Quick single-click buttons for immediate switching
        for (const auto& [type, label] : quickButtons) {
            pushButtonColors(currentType == type);
            if (ImGui::SmallButton(label)) {
                switchExample(type);
            }
            ImGui::PopStyleColor(3);
            ImGui::SameLine();
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.20f, 0.22f, 0.50f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.35f, 0.15f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
        if (ImGui::SmallButton(reloadLabel)) {
            reloadCurrent();
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(0.0f, 0.0f), ImVec2(viewport->Size.x - 20.0f, viewport->Size.y - ImGui::GetFrameHeight() - 10.0f)
    );
    if (beginDropdown(controlsLabel, "controls")) {
        if (activeExample && activeExample->hasControls()) {
            ImGui::PushItemWidth(std::clamp(viewport->Size.x * 0.4f, 120.0f, 220.0f));
            activeExample->renderControls();
            ImGui::PopItemWidth();
        } else {
            ImGui::TextDisabled("No controls for this example");
        }
        ImGui::EndMenu();
    }

    const std::string& rightText = compact ? brief : full;
    const float        rightX    = ImGui::GetWindowWidth() - ImGui::CalcTextSize(rightText.c_str()).x - 18.0f;
    if (rightX > ImGui::GetCursorPosX()) {
        ImGui::SetCursorPosX(rightX);
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", rightText.c_str());
    }

    ImGui::EndMainMenuBar();
}
