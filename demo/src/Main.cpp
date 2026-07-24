#include <SharkEngine/Application.hpp>
#include "layers/Example.hpp"

int main() {
    Application app{{ .width = 960, .height = 640, .title = "Example" }};

    app.push(std::make_unique<Example>());
    app.run();
}
