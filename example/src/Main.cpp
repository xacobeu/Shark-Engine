#include <memory>

#include "Application.hpp"
// #include "layers/SpinningCubeExample.hpp"
#include "layers/RayTracingExample.hpp"

int main() {
    Application app{
        Config{
            .width  = 960,
            .height = 640,
            .title  = "Example",
            .vsync  = false,
            .api    = GraphicsAPI::OpenGL,
        },
    };

    // app.push(std::make_unique<SpinningCubeExample>());
    app.push(std::make_unique<RayTracingExample>());
    app.run();
}
