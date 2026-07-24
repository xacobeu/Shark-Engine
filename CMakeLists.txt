cmake_minimum_required(VERSION 3.25)

cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE GLVND)

# Automatically use vcpkg toolchain if not defined
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(SharkEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detect vcpkg triplet based on platform
if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
elseif(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(VCPKG_TARGET_TRIPLET "arm64-osx" CACHE STRING "")
    else()
        set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "")
    endif()
else()
    set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "")
endif()

# Set vcpkg installed directory as prefix path
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")

# Find packages installed by vcpkg
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Common compiler options
set(SHARK_COMPILER_OPTIONS
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
        -Wall -Wextra -Wpedantic -Wold-style-cast -Wunused -Wcast-align -Wnon-virtual-dtor -Woverloaded-virtual
    >
)

add_subdirectory(engine)
add_subdirectory(demo)
