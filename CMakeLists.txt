cmake_minimum_required(VERSION 3.28)


cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE GLVND)

# Auto-detect vcpkg toolchain if VCPKG_ROOT is set in environment
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(SharkEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SHARK_ENABLE_PCH "Enable Precompiled Headers" ON)
option(SHARK_ENABLE_UNITY_BUILD "Enable Unity Builds" ON)

# Common compiler warnings across desktop and web
set(SHARK_COMPILER_OPTIONS
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
        -Wall -Wextra -Wpedantic -Wold-style-cast -Wunused -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual
    >
)

# Dependencies
if(NOT EMSCRIPTEN)
    set(VCPKG_INSTALLED_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed" CACHE PATH "vcpkg installed directory")

    if(WIN32)
        set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
    elseif(APPLE)
        set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "")
    else()
        set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "")
    endif()

    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")

    find_package(OpenGL REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(glad CONFIG REQUIRED)
    find_package(imgui CONFIG REQUIRED)
    find_package(nlohmann_json CONFIG REQUIRED)
    find_package(Lua REQUIRED)
    find_package(sol2 CONFIG REQUIRED)

else()
    # Emscripten / WebAssembly setup
    set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.deps" CACHE PATH "Directory for FetchContent sources")
    include(FetchContent)

    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(json)

    FetchContent_Declare(
        imgui_src
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        docking
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(imgui_src)

    add_library(glfw_dummy INTERFACE)
    target_compile_options(glfw_dummy INTERFACE -sUSE_GLFW=3)
    target_link_options(glfw_dummy INTERFACE -sUSE_GLFW=3)
    add_library(glfw ALIAS glfw_dummy)

    add_library(opengl_dummy INTERFACE)
    target_compile_options(opengl_dummy INTERFACE -sFULL_ES3=1)
    target_link_options(opengl_dummy INTERFACE -sFULL_ES3=1)
    add_library(OpenGL::GL ALIAS opengl_dummy)

    add_library(glad_dummy INTERFACE)
    target_compile_definitions(glad_dummy INTERFACE SHARK_NO_GLAD)
    add_library(glad::glad ALIAS glad_dummy)

    add_library(imgui STATIC
        ${imgui_src_SOURCE_DIR}/imgui.cpp
        ${imgui_src_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_src_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_src_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_src_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_src_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_src_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC
        ${imgui_src_SOURCE_DIR}
        ${imgui_src_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
    add_library(imgui::imgui ALIAS imgui)

    FetchContent_Declare(
        stb_src
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        master
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(stb_src)

    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE ${stb_src_SOURCE_DIR})

    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    FetchContent_Declare(
        lua
        GIT_REPOSITORY https://github.com/walterschell/Lua.git
        GIT_TAG        v5.4.6
        GIT_SHALLOW    ON
        PATCH_COMMAND  ${CMAKE_COMMAND} -D LUA_SRC_DIR=${FETCHCONTENT_BASE_DIR}/lua-src -P ${CMAKE_SOURCE_DIR}/cmake/PatchLua.cmake
    )
    set(LUA_ENABLE_SHARED OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(lua)

    FetchContent_Declare(
        sol2
        GIT_REPOSITORY https://github.com/ThePhD/sol2.git
        GIT_TAG        v3.5.0
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(sol2)
endif()


add_subdirectory(engine)
add_subdirectory(demo)