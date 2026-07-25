#include <SharkEngine/Application.hpp>
#include "layers/RayTracingExample.hpp"
#include "layers/SpinningCubeExample.hpp"

int main() {
    Application app{{ .width = 960, .height = 640, .title = "Example" }};

    // app.push(std::make_unique<SpinningCubeExample>());
    app.push(std::make_unique<RayTracingExample>());
    app.run();
}
