#include <memory>
#include <SharkEngine/Application.hpp>
#include <SharkEngine/ScriptLayer.hpp>
#include "layers/RayTracingExample.hpp"

int main() {
    Application app{
        Config{
            .width = 960,
            .height = 640,
            .title = "Example",
            .vsync = false,
            .api = GraphicsAPI::OpenGL
        }
    };

    //app.push(std::make_unique<ScriptLayer>("res/scripts/layer.lua"));
    app.push(std::make_unique<RayTracingExample>());
    app.run();
}
