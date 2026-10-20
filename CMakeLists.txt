cmake_minimum_required(VERSION 3.28)

cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE GLVND)

project(SharkEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
    find_package(OpenGL REQUIRED)
endif()

# Dependencies
add_subdirectory(external)

add_compile_options(
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra>
)

# Core Engine Target
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_library(SharkEngine ${ENGINE_SOURCES})
add_library(SharkEngine::SharkEngine ALIAS SharkEngine)

target_compile_features(SharkEngine PUBLIC cxx_std_26)

target_include_directories(SharkEngine PUBLIC src)
target_link_libraries(SharkEngine PUBLIC
    imgui::imgui
    glfw
    glad::glad
    OpenGL::GL
    stb
)

if(EMSCRIPTEN)
    target_link_options(SharkEngine INTERFACE
        "-sUSE_GLFW=3"
        "-sFULL_ES3=1"
        "-sUSE_WEBGL2=1"
        "-sMIN_WEBGL_VERSION=2"
        "-sMAX_WEBGL_VERSION=2"
        "-sALLOW_MEMORY_GROWTH=1"
        "--emrun"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/res@/res"
        "--shell-file" "${CMAKE_SOURCE_DIR}/res/web_template/index.html"
    )
endif()

# Demo Example (only when building top-level)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_subdirectory(example)
endif()
