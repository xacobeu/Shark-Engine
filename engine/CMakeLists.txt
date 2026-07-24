add_library(SharkEngine STATIC
    src/Application.cpp
    src/Window.cpp
    src/renderer/Camera.cpp
    src/renderer/Renderer.cpp
    src/renderer/geometry/Mesh.cpp
    src/renderer/geometry/MeshFactory.cpp
    src/renderer/GL/Shader.cpp
    src/renderer/GL/Vertex.cpp
)

target_compile_options(SharkEngine PRIVATE ${SHARK_COMPILER_OPTIONS})

target_include_directories(SharkEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(SharkEngine PUBLIC
    imgui::imgui
    glfw
    glad::glad
    OpenGL::GL
    implot::implot
    nlohmann_json::nlohmann_json
)

# Install targets
install(TARGETS SharkEngine
    EXPORT SharkEngineTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers preserving folder structure
install(DIRECTORY include/SharkEngine DESTINATION include)

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
