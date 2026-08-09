#pragma once

#include <filesystem>
#include <imgui.h>
#include <iostream>
#include <sol/sol.hpp>
#include <string>
#include <utility>

#include <SharkEngine/Application.hpp>
#include <SharkEngine/Layer.hpp>
#include <SharkEngine/renderer/Renderer.hpp>
#include <SharkEngine/renderer/geometry/Mesh.hpp>
#include <SharkEngine/renderer/geometry/MeshFactory.hpp>
#include <SharkEngine/renderer/scene/Scene.hpp>
#include <SharkEngine/renderer/scene/node_types/Entity.hpp>
#include <SharkEngine/renderer/scene/node_types/FlyingCamera.hpp>
#include <SharkEngine/renderer/scene/node_types/OrbitCamera.hpp>
#include <SharkEngine/util/Vector.hpp>

class ScriptLayer : public Layer {
  public:
    explicit ScriptLayer(std::string scriptPath) : scriptPath(std::move(scriptPath)) {}

    ~ScriptLayer() override = default;

    void onAttach() override;
    void onDetach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer &renderer) override;

    Scene &getScene() {
        return scene;
    }

  private:
    void initLua() {
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);
    }

    void bindAPI();
    void loadScript();
    void checkReload();

    std::string scriptPath;
    sol::state  lua;
    Scene       scene;

    [[maybe_unused]] std::filesystem::file_time_type lastModifiedTime;
    bool                                             hasAttached = false;
    [[maybe_unused]] float                           timeSinceLastCheck = 0.0f;
};
