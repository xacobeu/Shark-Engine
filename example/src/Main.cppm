import std;
import shark;

import examples.manager;

int main() {
    shark::Application app{{
        .width  = 1280,
        .height = 720,
        .title  = "SharkEngine - Examples",
        .vsync  = false,
    }};

    app.push(std::make_unique<ExampleManager>());
    app.run();
}
