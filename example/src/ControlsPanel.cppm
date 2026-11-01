module;

#include "imgui.h"

export module examples.ui;

import std;

export namespace examples {

// Whether the per-example "Controls" panel is shown. Toggled from the top bar.
inline bool controlsVisible = true;

// Opens the example's "Controls" panel, sized to fit small screens. Returns false when the
// panel is hidden from the top bar; only call ImGui::End() when this returned true.
bool beginControls(ImVec2 defaultSize) {
    if (!controlsVisible) {
        return false;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float          margin   = 10.0f;
    const float          top      = ImGui::GetFrameHeight() + margin;
    const ImVec2         maxSize(
        std::max(viewport->Size.x - (2.0f * margin), 100.0f), std::max(viewport->Size.y - top - margin, 100.0f)
    );

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + margin, viewport->Pos.y + top), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(
        ImVec2(std::min(defaultSize.x, maxSize.x), std::min(defaultSize.y, maxSize.y)), ImGuiCond_FirstUseEver
    );
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), maxSize);
    ImGui::Begin("Controls");
    return true;
}

} // namespace examples
