# SharkEngine

A cross-platform C++26 rendering engine.

## Project Structure

* **`src/`**: The engine.
* **`res/`**: Default engine assets.
* **`example/`**: Example application demonstrating engine features.

## Prerequisites

* **C++26 Compiler**
* **CMake**

(optional):
* **make**
* **Ninja**
* **Emscripten SDK** 

## Running the Demo

```bash
# Check the help menu
make
```


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

#### Building Your Game
* **Native Desktop**:
  ```bash
  cmake -B build
  cmake --build build
  ```
* **WebAssembly (Browser)**:
  ```bash
  emcmake cmake -B build-web
  cmake --build build-web
  ```
  SharkEngine automatically configures WebGL, preloads engine shaders, and outputs `MyGame.html`.

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
Include headers and run:
```cpp
#include "Application.hpp"

int main() {
      Application app{{ .width = 960, .height = 640, .title = "My Game" }};
      app.run();
}
```

