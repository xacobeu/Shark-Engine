cmake_minimum_required(VERSION 3.30)

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
    $<$<AND:$<NOT:$<BOOL:${EMSCRIPTEN}>>,$<CONFIG:Release>>:-march=native>
    $<$<AND:$<NOT:$<BOOL:${EMSCRIPTEN}>>,$<CONFIG:RelWithDebInfo>>:-march=native>
    $<$<BOOL:${EMSCRIPTEN}>:-msimd128>
)

include(cmake/StdModule.cmake)

# Core Engine Target
file(GLOB_RECURSE ENGINE_MODULES CONFIGURE_DEPENDS "src/*.cppm")
file(GLOB_RECURSE ENGINE_INTERNAL_MODULES CONFIGURE_DEPENDS
    "src/renderer/backends/opengl/*.cppm"
    "src/renderer/backends/RenderDeviceFactory.cppm"
)
list(REMOVE_ITEM ENGINE_MODULES ${ENGINE_INTERNAL_MODULES})
add_library(SharkEngine)
add_library(SharkEngine::SharkEngine ALIAS SharkEngine)
target_sources(SharkEngine
    PUBLIC FILE_SET CXX_MODULES BASE_DIRS src FILES ${ENGINE_MODULES}
    PRIVATE FILE_SET engine_impl TYPE CXX_MODULES BASE_DIRS src FILES ${ENGINE_INTERNAL_MODULES}
)

target_link_libraries(SharkEngine PUBLIC
    shark_std
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
    set_property(TARGET SharkEngine APPEND PROPERTY
        INTERFACE_LINK_DEPENDS "${CMAKE_SOURCE_DIR}/res/web_template/index.html"
    )
elseif(NOT WIN32)
    find_package(TBB QUIET)
    if(TBB_FOUND)
        target_link_libraries(shark_std PUBLIC TBB::tbb)
    else()
        target_compile_definitions(shark_std PUBLIC
            SHARK_NO_PARALLEL_EXECUTION
            _PSTL_PAR_BACKEND_SERIAL
        )
    endif()
endif()

# Demo Example (only when building top-level)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_subdirectory(example)
endif()
