add_library(SharkEngine)

target_compile_options(SharkEngine PRIVATE ${SHARK_COMPILER_OPTIONS})

target_compile_definitions(SharkEngine PUBLIC SOL_ALL_SAFETIES_ON=1)

if(EMSCRIPTEN)
    set_target_properties(SharkEngine PROPERTIES UNITY_BUILD OFF)
else()
    set_target_properties(SharkEngine PROPERTIES UNITY_BUILD ON)
endif()

if(NOT EMSCRIPTEN)
    target_precompile_headers(SharkEngine PRIVATE <glad/glad.h>)
endif()

target_precompile_headers(SharkEngine PRIVATE
    <memory>
    <vector>
    <string>
    <unordered_map>
    <functional>
    <stdexcept>
    <iostream>
    <filesystem>
    <imgui.h>
    <GLFW/glfw3.h>
    <nlohmann/json.hpp>
    <sol/sol.hpp>
)

target_include_directories(SharkEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_sources(SharkEngine
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS src
        FILES
            src/SharkEngine/Application.hpp
            src/SharkEngine/Layer.hpp
            src/SharkEngine/ScriptLayer.hpp
            src/SharkEngine/Window.hpp
            src/SharkEngine/renderer/Renderer.hpp
            src/SharkEngine/renderer/backends/IIndexBuffer.hpp
            src/SharkEngine/renderer/backends/IRenderDevice.hpp
            src/SharkEngine/renderer/backends/IShader.hpp
            src/SharkEngine/renderer/backends/ITexture.hpp
            src/SharkEngine/renderer/backends/IVertexBuffer.hpp
            src/SharkEngine/renderer/backends/opengl/IndexBuffer.hpp
            src/SharkEngine/renderer/backends/opengl/Object.hpp
            src/SharkEngine/renderer/backends/opengl/RenderDevice.hpp
            src/SharkEngine/renderer/backends/opengl/Shader.hpp
            src/SharkEngine/renderer/backends/opengl/Texture.hpp
            src/SharkEngine/renderer/backends/opengl/VertexBuffer.hpp
            src/SharkEngine/renderer/geometry/Mesh.hpp
            src/SharkEngine/renderer/geometry/MeshFactory.hpp
            src/SharkEngine/renderer/geometry/Vertex.hpp
            src/SharkEngine/renderer/scene/Node.hpp
            src/SharkEngine/renderer/scene/Scene.hpp
            src/SharkEngine/renderer/scene/node_types/Camera.hpp
            src/SharkEngine/renderer/scene/node_types/Entity.hpp
            src/SharkEngine/renderer/scene/node_types/FlyingCamera.hpp
            src/SharkEngine/renderer/scene/node_types/Light.hpp
            src/SharkEngine/renderer/scene/node_types/OrbitCamera.hpp
            src/SharkEngine/util/Matrix4.hpp
            src/SharkEngine/util/Vector.hpp
    PRIVATE
        src/SharkEngine/Application.cpp
        src/SharkEngine/Window.cpp
        src/SharkEngine/ScriptLayer.cpp
        src/SharkEngine/renderer/Renderer.cpp
        src/SharkEngine/renderer/backends/IRenderDevice.cpp
        src/SharkEngine/renderer/backends/opengl/IndexBuffer.cpp
        src/SharkEngine/renderer/backends/opengl/RenderDevice.cpp
        src/SharkEngine/renderer/backends/opengl/Shader.cpp
        src/SharkEngine/renderer/backends/opengl/Texture.cpp
        src/SharkEngine/renderer/backends/opengl/VertexBuffer.cpp
        src/SharkEngine/renderer/geometry/Mesh.cpp
        src/SharkEngine/renderer/geometry/MeshFactory.cpp
        src/SharkEngine/renderer/scene/FlyingCamera.cpp
        src/SharkEngine/renderer/scene/OrbitCamera.cpp
)

target_link_libraries(SharkEngine PUBLIC
    imgui::imgui
    glfw
    glad::glad
    OpenGL::GL
    nlohmann_json::nlohmann_json
    sol2::sol2
)

if(EMSCRIPTEN)
    target_link_libraries(SharkEngine PUBLIC stb lua_static)
else()
    if(TARGET Lua::Lua)
        target_link_libraries(SharkEngine PUBLIC Lua::Lua)
    else()
        target_link_libraries(SharkEngine PUBLIC ${LUA_LIBRARIES})
        target_include_directories(SharkEngine PUBLIC $<BUILD_INTERFACE:${LUA_INCLUDE_DIR}>)
    endif()
endif()

# Install targets (disabled for WebAssembly)
if(NOT EMSCRIPTEN)
    install(TARGETS SharkEngine
        EXPORT SharkEngineTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        FILE_SET HEADERS DESTINATION include
    )

    # Configure package config for find_package
    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SharkEngineConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/SharkEngineConfig.cmake"
        INSTALL_DESTINATION lib/cmake/SharkEngine
    )

    install(EXPORT SharkEngineTargets
        FILE SharkEngineTargets.cmake
        NAMESPACE SharkEngine::
        DESTINATION lib/cmake/SharkEngine
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/SharkEngineConfig.cmake"
        DESTINATION lib/cmake/SharkEngine
    )
endif()

