# Vendor dependencies for SharkEngine

# 1. GLFW
if(EMSCRIPTEN)
    add_library(glfw INTERFACE)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
else()
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(glfw)
endif()

# 2. GLAD
if(EMSCRIPTEN)
    add_library(glad INTERFACE)
    target_compile_definitions(glad INTERFACE SHARK_NO_GLAD)
    add_library(glad::glad ALIAS glad)
else()
    add_library(glad STATIC
        glad/src/glad.c
    )
    target_include_directories(glad PUBLIC
        glad/include
    )
    target_compile_options(glad PRIVATE
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
        $<$<CXX_COMPILER_ID:MSVC>:/W0>
    )
    add_library(glad::glad ALIAS glad)
endif()

# 3. OpenGL (Emscripten mapping)
if(EMSCRIPTEN)
    add_library(opengl_dummy INTERFACE)
    target_link_options(opengl_dummy INTERFACE -sFULL_ES3=1)
    add_library(OpenGL::GL ALIAS opengl_dummy)
endif()

# 4. ImGui
add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)
target_compile_options(imgui PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
)
target_include_directories(imgui SYSTEM PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
add_library(imgui::imgui ALIAS imgui)

# 5. stb
add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/stb
)

