add_library(SharkEngine)

target_compile_options(SharkEngine PRIVATE ${SHARK_COMPILER_OPTIONS})

target_compile_definitions(SharkEngine PUBLIC SOL_ALL_SAFETIES_ON=1)

if(SHARK_ENABLE_UNITY_BUILD)
    set_target_properties(SharkEngine PROPERTIES UNITY_BUILD OFF)
endif()

# We specify all .cppm module files in the PUBLIC CXX_MODULES file set
target_sources(SharkEngine
    PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS src
        FILES
            src/util/Math.cppm
            src/renderer/geometry/Vertex.cppm
            src/renderer/geometry/Mesh.cppm
            src/renderer/geometry/MeshFactory.cppm
            src/renderer/backends/RenderDeviceAbstractions.cppm
            src/renderer/backends/opengl/GLObject.cppm
            src/renderer/backends/opengl/GLVertexBuffer.cppm
            src/renderer/backends/opengl/GLIndexBuffer.cppm
            src/renderer/backends/opengl/GLShader.cppm
            src/renderer/backends/opengl/GLTexture.cppm
            src/renderer/backends/opengl/GLRenderDevice.cppm
            src/renderer/scene/Node.cppm
            src/renderer/scene/node_types/SceneNodes.cppm
            src/renderer/scene/Scene.cppm
            src/renderer/Renderer.cppm
            src/Layer.cppm
            src/Window.cppm
            src/ScriptLayer.cppm
            src/Application.cppm
            src/SharkEngine.cppm
)

target_sources(SharkEngine PRIVATE
    src/renderer/scene/node_types/SceneNodes.cpp
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
        FILE_SET CXX_MODULES DESTINATION include
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
