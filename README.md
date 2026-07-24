# SharkEngine

A lightweight and cross-platform C++20 rendering engine built using OpenGL, GLFW, glad, and ImGui.

## Project Structure

* **`engine/`**: The core rendering engine library target.
* **`demo/`**: A sandbox application demonstrating engine features.
* **`engine/res/`**: Default assets used by the engine.

## Prerequisites

* **C++20 Compiler**
* **CMake**
* **vcpkg**

(optional, only for Web build):
- **just**
- **Ninja**
- **Emscripten SDK** 

## Running the Demo

### Desktop Build
```bash
# Build.
just build desktop
# Build and Run.
just run desktop
```

### WebAssembly Build
```bash
# Build.
just build web
# Build and Run.
just run web
```

Use `just clean` to clean all built artifacts.

## Usage as a Dependency
You can configure CMake to automatically download, configure, and compile `SharkEngine` at build time from its GitHub repository:
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

### Alternatively
Add `SharkEngine` as a submodule to your project or download it manually into your project (e.g., in a `thirdparty/` folder):
```bash
git submodule add https://github.com/xacobeu/SharkEngine.git thirdparty/SharkEngine
```
Link against the library target in your project's `CMakeLists.txt`:
```cmake
   # Make sure you configure CMake with the vcpkg toolchain file:
   # cmake -B build -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"

   add_subdirectory(thirdparty/SharkEngine)

   add_executable(MyGame src/Main.cpp)
   target_link_libraries(MyGame PRIVATE SharkEngine::SharkEngine)
   ```
Include headers and run:
```cpp
#include <SharkEngine/Application.hpp>

int main() {
      Application app{{ .width = 960, .height = 640, .title = "My Game" }};
      app.run();
}
```

