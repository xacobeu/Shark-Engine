# SharkEngine

A cross-platform C++26 rendering engine with an OpenGL backend.

> [!WARNING]
> **Status is experimental:** The API is unstable and may change between commits.


## Project Structure

```text
.
├── example/      // Engine Demos
├── external/     // Third Party Code
├── res/          // Engine Assets 
└── src/          // Engine Code
```

## Getting Started
### Prerequisites

* **Clang 21+**
* **A standard library that ships the `std` module**: libstdc++ 15+, MSVC STL 17.6+, or libc++ (Emscripten's works out of the box)
* **CMake 3.30+**
* **Ninja**

(optional):
* **make**
* **Emscripten SDK** 

### Running the Demos

Find detailed command instructions by running `make help`.
```bash
make run
```

### Usage as a Dependency
You can configure CMake to automatically download, configure, and compile the engine at build time from its GitHub repository:
```cmake
include(FetchContent)

FetchContent_Declare(
    SharkEngine
    GIT_REPOSITORY https://github.com/xacobeu/SharkEngine.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(SharkEngine)

add_executable(MyGame src/Main.cpp)
target_link_libraries(MyGame PRIVATE SharkEngine::SharkEngine)
```

### Building Your Game
```bash
# Native (Desktop)
cmake -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++
cmake --build build
```
```bash
# WASM (Web)
emcmake cmake -B build-web -G Ninja
cmake --build build-web
```

### Alternatively
Add `SharkEngine` as a submodule to your project or download it manually into your project (e.g., in a `thirdparty/` folder):
```bash
git submodule add https://github.com/xacobeu/SharkEngine.git thirdparty/SharkEngine
```
Link against the library target in your project's `CMakeLists.txt`:
```cmake
add_subdirectory(thirdparty/SharkEngine)

add_executable(MyGame src/Main.cpp)
target_link_libraries(MyGame PRIVATE SharkEngine::SharkEngine)
```
Using the engine (See [examples](example/src/) for more detail):
```cpp
import std;
import shark;

class ExampleLayer : public shark::Layer {
public:
    void onUpdate(float deltaTime) override { /* game logic */ }
    void onRender(shark::Renderer& renderer) override { /* draw */ }
};

int main() {
    shark::Application app{{ 
        .width = 960, 
        .height = 640, 
        .title = "My Game" 
    }};
    app.push(std::make_unique<ExampleLayer>());
    app.run();
}

```

## External Code

Vendored in [external/](external/):
- imgui
- stb
- glad (desktop only)
- glfw (desktop only)

System:
- OpenGL (desktop only)
- TBB (optional for linux to enable multithreading)