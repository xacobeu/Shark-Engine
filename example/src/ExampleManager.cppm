module;

#include "imgui.h"

export module examples.manager;

import std;

import examples.fast_multipole_method;
import examples.fluid_simulation;
import examples.molecular_dynamics;
import examples.ray_tracing;
import examples.spinning_cube;

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
    static std::unique_ptr<Layer> createExample(ExampleType type);

    ExampleType currentType = ExampleType::FluidSimulation;
    ExampleType pendingType = ExampleType::FluidSimulation;
    bool        needsReload = false;

    std::unique_ptr<Layer> activeExample;
};

ExampleManager::ExampleManager() {
    SetupForestGreenStyle();
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

std::unique_ptr<Layer> ExampleManager::createExample(ExampleType type) {
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

    // Quick single-click buttons for immediate switching
    auto renderQuickButton = [this](ExampleType type, const char* label) {
        bool isActive = (currentType == type);
        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.45f, 0.85f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.55f, 0.95f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.20f, 0.50f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.35f, 0.70f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.80f, 1.0f));
        }

        if (ImGui::SmallButton(label)) {
            switchExample(type);
        }

        ImGui::PopStyleColor(3);
        ImGui::SameLine();
    };

    renderQuickButton(ExampleType::FluidSimulation, "Fluid");
    renderQuickButton(ExampleType::FastMultipoleMethod, "FMM");
    renderQuickButton(ExampleType::MolecularDynamics, "Molecular");
    renderQuickButton(ExampleType::SpinningCube, "Cube");
    renderQuickButton(ExampleType::RayTracing, "Raytracer");

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.20f, 0.22f, 0.50f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.35f, 0.15f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
    if (ImGui::SmallButton("Reload (R)")) {
        reloadCurrent();
    }
    ImGui::PopStyleColor(3);

    // Right-aligned status and framerate
    ImGuiIO& io  = ImGui::GetIO();
    float    fps = io.Framerate;
    float    ms  = (fps > 0.0f) ? (1000.0f / fps) : 0.0f;

    std::string rightText = std::format("{} | {:.1f} FPS ({:.2f} ms)", getExampleName(currentType), fps, ms);
    float       textWidth = ImGui::CalcTextSize(rightText.c_str()).x;
    float       rightX    = ImGui::GetWindowWidth() - textWidth - 18.0f;
    if (rightX > ImGui::GetCursorPosX()) {
        ImGui::SetCursorPosX(rightX);
    }

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", rightText.c_str());

    ImGui::EndMainMenuBar();
}
