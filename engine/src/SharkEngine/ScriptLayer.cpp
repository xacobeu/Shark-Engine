#include <SharkEngine/ScriptLayer.hpp>

void ScriptLayer::onAttach() {
    initLua();
    bindAPI();

    try {
        if (std::filesystem::exists(scriptPath)) {
            lastModifiedTime = std::filesystem::last_write_time(scriptPath);
        }
    } catch (...) { }

    app->setScene(&scene);

    loadScript();

    sol::protected_function attachFn = lua["onAttach"];
    if (attachFn.valid()) {
        auto result = attachFn();
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "[LUA ERROR] onAttach failed: " << err.what() << std::endl;
        }
    }

    hasAttached = true;
}

void ScriptLayer::onDetach() {
    sol::protected_function detachFn = lua["onDetach"];
    if (detachFn.valid()) {
        auto result = detachFn();
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "[LUA ERROR] onDetach failed: " << err.what() << std::endl;
        }
    }
}

void ScriptLayer::onUpdate(float deltaTime) {
    checkReload();

    sol::protected_function updateFn = lua["onUpdate"];
    if (updateFn.valid()) {
        auto result = updateFn(deltaTime);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "[LUA ERROR] onUpdate failed: " << err.what() << std::endl;
        }
    }
}

void ScriptLayer::onRender(Renderer &renderer) {
    sol::protected_function renderFn = lua["onRender"];
    if (renderFn.valid()) {
        auto result = renderFn(renderer);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "[LUA ERROR] onRender failed: " << err.what() << std::endl;
        }
    } else {
        renderer.render(scene, app->getWidth(), app->getHeight());
    }
}

void ScriptLayer::bindAPI() {
    // Bind Globals / Context
    lua["app"] = app;
    lua["scene"] = &scene;

    // Vector3
    lua.new_usertype<Vector3>(
        "Vector3", sol::constructors<Vector3(), Vector3(float, float, float)>(), "x", &Vector3::x, "y", &Vector3::y, "z", &Vector3::z,
        sol::meta_function::addition, [](const Vector3 &a, const Vector3 &b) { return a + b; }, sol::meta_function::subtraction,
        [](const Vector3 &a, const Vector3 &b) { return a - b; }, sol::meta_function::multiplication,
        sol::overload([](const Vector3 &a, float s) { return a * s; }, [](const Vector3 &a, const Vector3 &b) { return a * b; }),
        sol::meta_function::division,
        sol::overload([](const Vector3 &a, float s) { return a / s; }, [](const Vector3 &a, const Vector3 &b) { return a / b; }));

    // Vector4
    lua.new_usertype<Vector4>("Vector4", sol::constructors<Vector4(), Vector4(float, float, float, float)>(), "x", &Vector4::x, "y",
                              &Vector4::y, "z", &Vector4::z, "w", &Vector4::w);

    // Node
    lua.new_usertype<Node>(
        "Node", "getPosition", &Node::getPosition, "setPosition", [](Node &self, const Vector3 &pos) { self.setPosition(pos); },
        "getRotation", &Node::getRotation, "setRotation", [](Node &self, const Vector3 &rot) { self.setRotation(rot); }, "getScale",
        &Node::getScale, "setScale", [](Node &self, const Vector3 &scl) { self.setScale(scl); });

    // Entity
    lua.new_usertype<Entity>(
        "Entity", sol::base_classes, sol::bases<Node>(), "setColor", [](Entity &self, const Vector4 &col) { self.setColor(col); },
        "getColor", &Entity::getColor, "setMesh", [](Entity &self, std::shared_ptr<Mesh> mesh) { self.setMesh(mesh); });

    // Mesh
    lua.new_usertype<Mesh>("Mesh");

    // MeshFactory
    auto meshFactory = lua["MeshFactory"].get_or_create<sol::table>();
    meshFactory["cube"] = []() { return std::make_shared<Mesh>(MeshFactory::cube()); };

    // Camera
    lua.new_usertype<Camera>("Camera", sol::base_classes, sol::bases<Node>());

    // FlyingCamera
    lua.new_usertype<FlyingCamera>(
        "FlyingCamera", sol::base_classes, sol::bases<Camera, Node>(), "setPosition",
        [](FlyingCamera &self, const Vector3 &pos) {
            self.setPosition(pos);
            return &self;
        },
        "setRotation",
        [](FlyingCamera &self, const Vector3 &rot) {
            self.setRotation(rot);
            return &self;
        });

    // OrbitCamera
    lua.new_usertype<OrbitCamera>("OrbitCamera", sol::base_classes, sol::bases<Camera, Node>());

    // Scene
    lua.new_usertype<Scene>(
        "Scene", "addEntity", [](Scene &self) -> Entity * { return self.add<Entity>(); }, "addFlyingCamera",
        [](Scene &self) -> FlyingCamera * { return self.add<FlyingCamera>(); }, "addOrbitCamera",
        [](Scene &self) -> OrbitCamera * { return self.add<OrbitCamera>(); }, "setActiveCamera", &Scene::setActiveCamera);

    // Application
    lua.new_usertype<Application>("Application", "getWidth", &Application::getWidth, "getHeight", &Application::getHeight, "getElapsedTime",
                                  &Application::getElapsedTime, "getDeltaTime", &Application::getDeltaTime, "isKeyPressed",
                                  &Application::isKeyPressed, "isMouseButtonPressed", &Application::isMouseButtonPressed, "setScene",
                                  &Application::setScene);

    // Renderer
    lua.new_usertype<Renderer>("Renderer", "render", &Renderer::render);

    // ImGui
    auto imgui = lua["ImGui"].get_or_create<sol::table>();
    imgui["Begin"] = [](const std::string &name) { ImGui::Begin(name.c_str()); };
    imgui["End"] = []() { ImGui::End(); };
    imgui["Text"] = [](const std::string &text) { ImGui::TextUnformatted(text.c_str()); };
    imgui["Separator"] = []() { ImGui::Separator(); };
}

void ScriptLayer::checkReload() {
#ifndef __EMSCRIPTEN__
    timeSinceLastCheck += app->getDeltaTime();
    if (timeSinceLastCheck < 0.25f)
        return;
    timeSinceLastCheck = 0.0f;

    try {
        if (std::filesystem::exists(scriptPath)) {
            auto writeTime = std::filesystem::last_write_time(scriptPath);
            if (writeTime > lastModifiedTime) {
                lastModifiedTime = writeTime;
                std::cout << "[INFO] Script modified. Reloading " << scriptPath << "..." << std::endl;

                scene = Scene();
                app->setScene(&scene);

                loadScript();

                sol::protected_function attachFn = lua["onAttach"];
                if (attachFn.valid()) {
                    auto result = attachFn();
                    if (!result.valid()) {
                        sol::error err = result;
                        std::cerr << "[LUA ERROR] onAttach failed after reload: " << err.what() << std::endl;
                    }
                }
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "[ERROR] Exception during script reload check: " << e.what() << std::endl;
    }
#endif
}

void ScriptLayer::loadScript() {
    auto result = lua.safe_script_file(scriptPath, sol::script_pass_on_error);
    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[LUA ERROR] Script load failed: " << err.what() << std::endl;
    }
}
