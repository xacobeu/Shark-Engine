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
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(SHARK_ENABLE_PCH "Enable Precompiled Headers" ON)
option(SHARK_ENABLE_UNITY_BUILD "Enable Unity Builds" ON)
option(SHARK_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# Strict compiler warnings for code quality, safety, and modern C++ style
set(SHARK_MSVC_WARNINGS
    /W4                  # Level 4 warnings
    /permissive-         # Enforce standard conformance
    /external:anglebrackets # Treat <...> headers as external
    /external:W0         # Ignore warnings in external headers
    /w14242              # conversion from 'type1' to 'type2', possible loss of data
    /w14254              # conversion from 'type1:field_bits' to 'type2:field_bits'
    /w14263              # member function does not override base class virtual function
    /w14265              # class has virtual functions, but destructor is not virtual
    /w14287              # unsigned/negative constant mismatch
    /w14296              # expression is always true/false
    /w14311              # pointer truncation from 'type1' to 'type2'
    /w14389              # signed/unsigned mismatch in comparison
    /w14545              # comma expression missing argument list
    /w14546              # function call before comma missing argument list
    /w14547              # operator before comma has no effect
    /w14549              # operator before comma has no effect
    /w14555              # expression has no effect
    /w14619              # pragma warning: no warning number
    /w14640              # thread-unsafe static member initialization
    /w14701              # potentially uninitialized local variable used
    /w14702              # unreachable code
    /w14706              # assignment within conditional expression
    /w14826              # Conversion from 'type1' to 'type2' is sign-extended
    /w14905              # wide string literal cast to LPSTR
    /w14906              # string literal cast to LPWSTR
    /w14928              # illegal copy-initialization
)

set(SHARK_GCC_CLANG_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow             # Warn when a local variable shadows another
    -Wnon-virtual-dtor   # Class with virtual functions but non-virtual destructor
    -Wold-style-cast     # C-style casts in C++
    -Wcast-align         # Cast increases required alignment
    -Wunused             # Unused variables / functions
    -Woverloaded-virtual # Overloaded virtual function hides base virtual function
    -Wconversion         # Implicit type conversions
    -Wsign-conversion    # Conversion between signed and unsigned types
    -Wnull-dereference   # Potential null pointer dereference
    -Wdouble-promotion   # Implicit float to double promotion
    -Wformat=2           # Security warnings for printf/scanf format strings
    -Wimplicit-fallthrough # Unannotated fallthrough in switch cases
    -Wextra-semi         # Redundant semicolons
)

set(SHARK_COMPILER_OPTIONS
    $<$<CXX_COMPILER_ID:MSVC>:${SHARK_MSVC_WARNINGS}>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${SHARK_GCC_CLANG_WARNINGS}>
    $<$<BOOL:${SHARK_WARNINGS_AS_ERRORS}>:$<$<CXX_COMPILER_ID:MSVC>:/WX>$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>>
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
    include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/.deps")
    include(FetchContent)

    FetchContent_Declare(
        json
        SYSTEM
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(json)

    FetchContent_Declare(
        imgui_src
        SYSTEM
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        docking
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(imgui_src)

    add_library(glfw_dummy INTERFACE)
    target_link_options(glfw_dummy INTERFACE -sUSE_GLFW=3)
    add_library(glfw ALIAS glfw_dummy)

    add_library(opengl_dummy INTERFACE)
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
    target_compile_options(imgui PRIVATE
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
        $<$<CXX_COMPILER_ID:MSVC>:/W0>
    )
    target_include_directories(imgui SYSTEM PUBLIC
        ${imgui_src_SOURCE_DIR}
        ${imgui_src_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
    add_library(imgui::imgui ALIAS imgui)

    FetchContent_Declare(
        stb_src
        SYSTEM
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        master
        GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(stb_src)

    add_library(stb INTERFACE)
    target_include_directories(stb SYSTEM INTERFACE ${stb_src_SOURCE_DIR})

    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    FetchContent_Declare(
        lua
        SYSTEM
        GIT_REPOSITORY https://github.com/walterschell/Lua.git
        GIT_TAG        v5.4.6
        GIT_SHALLOW    ON
        PATCH_COMMAND  ${CMAKE_COMMAND} -D LUA_SRC_DIR=${FETCHCONTENT_BASE_DIR}/lua-src -P ${CMAKE_SOURCE_DIR}/cmake/PatchLua.cmake
    )
    set(LUA_ENABLE_SHARED OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(lua)

    if(TARGET lua_static)
        target_compile_options(lua_static PRIVATE
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
            $<$<CXX_COMPILER_ID:MSVC>:/W0>
        )
    endif()

    FetchContent_Declare(
        sol2
        SYSTEM
        GIT_REPOSITORY https://github.com/ThePhD/sol2.git
        GIT_TAG        v3.5.0
        GIT_SHALLOW    ON
        PATCH_COMMAND  ${CMAKE_COMMAND} -D SOL_SRC_DIR=${FETCHCONTENT_BASE_DIR}/sol2-src -P ${CMAKE_SOURCE_DIR}/cmake/PatchSol.cmake
    )
    FetchContent_MakeAvailable(sol2)
endif()


add_subdirectory(engine)
add_subdirectory(demo)