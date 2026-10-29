
import std;

import examples;
import shark;

using namespace shark;

int main() {
    Application app{
        Config{
            .width  = 1280,
            .height = 720,
            .title  = "SharkEngine - Examples",
            .vsync  = false,
            .api    = GraphicsAPI::OpenGL,
        },
    };

    app.push(std::make_unique<ExampleManager>());
    app.run();
}
