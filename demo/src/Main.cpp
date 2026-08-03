#include <SharkEngine/Application.hpp>
#include "layers/RayTracingExample.hpp"
#include "layers/SpinningCubeExample.hpp"
#include "layers/BoxExample.hpp"

int main() {
    Application app{{ .width = 960, .height = 640, .title = "Example", .vsync = false }};

    // app.push(std::make_unique<BoxExample>());
    // app.push(std::make_unique<SpinningCubeExample>());
    app.push(std::make_unique<RayTracingExample>());
    app.run();
}
