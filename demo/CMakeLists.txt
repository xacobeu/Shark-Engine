add_executable(SharkEngineDemo
    src/Main.cpp
    src/layers/SpinningCubeExample.cpp
    src/layers/RayTracingExample.cpp
)

target_compile_options(SharkEngineDemo PRIVATE ${SHARK_COMPILER_OPTIONS})
target_compile_features(SharkEngineDemo PRIVATE cxx_std_20)


set_target_properties(SharkEngineDemo PROPERTIES UNITY_BUILD ON)

target_precompile_headers(SharkEngineDemo PRIVATE
    <memory>
    <vector>
    <string>
    <SharkEngine/Application.hpp>
    <SharkEngine/renderer/Renderer.hpp>
)

target_link_libraries(SharkEngineDemo PRIVATE
    SharkEngine
)

if(EMSCRIPTEN)
    # Shaders and scripts are preloaded into the generated .data file. Make the link step
    # depend on them so an edit refreshes the browser artifact.
    set_property(TARGET SharkEngineDemo APPEND PROPERTY LINK_DEPENDS
        "${SharkEngine_SOURCE_DIR}/engine/res/shaders/default.vert.glsl"
        "${SharkEngine_SOURCE_DIR}/engine/res/shaders/default.frag.glsl"
        "${SharkEngine_SOURCE_DIR}/engine/res/shaders/raytrace.vert.glsl"
        "${SharkEngine_SOURCE_DIR}/engine/res/shaders/raytrace.frag.glsl"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/layers/layer.lua"
    )

    # Output SharkEngineDemo.html, SharkEngineDemo.js, and SharkEngineDemo.wasm
    set_target_properties(SharkEngineDemo PROPERTIES SUFFIX ".html")

    target_link_options(SharkEngineDemo PRIVATE
        "-sUSE_GLFW=3"
        "-sFULL_ES3=1"
        "-sUSE_WEBGL2=1"
        "-sMIN_WEBGL_VERSION=2"
        "-sMAX_WEBGL_VERSION=2"
        "-sALLOW_MEMORY_GROWTH=1"
        "SHELL:--preload-file ${SharkEngine_SOURCE_DIR}/engine/res@/res"
        "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/src/layers/layer.lua@res/scripts/layer.lua"
        "--shell-file" "${CMAKE_CURRENT_SOURCE_DIR}/web/index.html"
    )

else()
    add_custom_target(CopyEngineResources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SharkEngine_SOURCE_DIR}/engine/res"
            "$<TARGET_FILE_DIR:SharkEngineDemo>/res"
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:SharkEngineDemo>/res/scripts"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/src/layers/layer.lua"
            "$<TARGET_FILE_DIR:SharkEngineDemo>/res/scripts/layer.lua"
        COMMENT "Copying engine and demo resources to output directory"
    )
    add_dependencies(SharkEngineDemo CopyEngineResources)

    # Enable app-local dependency copying for vcpkg on Windows
    if(WIN32)
        set(VCPKG_APPLOCAL_DEPS ON)
    endif()

    install(TARGETS SharkEngineDemo RUNTIME DESTINATION bin)
    install(DIRECTORY "${SharkEngine_SOURCE_DIR}/engine/res/" DESTINATION bin/res)
endif()
