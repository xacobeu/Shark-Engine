#pragma once

#include <sol/sol.hpp>
#include <imgui.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <utility>

#include <SharkEngine/Layer.hpp>
#include <SharkEngine/renderer/scene/Scene.hpp>
#include <SharkEngine/renderer/scene/node_types/Entity.hpp>
#include <SharkEngine/renderer/scene/node_types/FlyingCamera.hpp>
#include <SharkEngine/renderer/scene/node_types/OrbitCamera.hpp>
#include <SharkEngine/util/Vector.hpp>
#include <SharkEngine/renderer/Renderer.hpp>
#include <SharkEngine/renderer/geometry/MeshFactory.hpp>
#include <SharkEngine/renderer/geometry/Mesh.hpp>
#include <SharkEngine/Application.hpp>

class ScriptLayer : public Layer {
public:
    explicit ScriptLayer(std::string scriptPath) : scriptPath(std::move(scriptPath)) {
#ifndef __EMSCRIPTEN__
        if (this->scriptPath == "res/scripts/layer.lua") {
            if (std::filesystem::exists("../../../engine/res/scripts/layer.lua")) {
                this->scriptPath = "../../../engine/res/scripts/layer.lua";
            } else if (std::filesystem::exists("engine/res/scripts/layer.lua")) {
                this->scriptPath = "engine/res/scripts/layer.lua";
            }
        }
#endif
    }

    ~ScriptLayer() override = default;

    void onAttach() override;
    void onDetach() override;
    void onUpdate(float deltaTime) override;
    void onRender(Renderer& renderer) override;

    Scene& getScene() { return scene; }

private:
    void initLua() {
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);
    }

    void bindAPI();
    void loadScript();
    void checkReload();

    std::string scriptPath;
    sol::state lua;
    Scene scene;
    
    [[maybe_unused]] std::filesystem::file_time_type lastModifiedTime;
    bool hasAttached = false;
    [[maybe_unused]] float timeSinceLastCheck = 0.0f;
};
