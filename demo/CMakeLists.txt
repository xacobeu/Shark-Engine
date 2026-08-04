add_executable(SharkEngineDemo
    src/Main.cpp
    src/layers/SpinningCubeExample.cpp
    src/layers/RayTracingExample.cpp
)

target_compile_options(SharkEngineDemo PRIVATE ${SHARK_COMPILER_OPTIONS})
target_compile_features(SharkEngineDemo PRIVATE cxx_std_20)


if(SHARK_ENABLE_UNITY_BUILD)
    set_target_properties(SharkEngineDemo PROPERTIES UNITY_BUILD ON)
endif()

target_link_libraries(SharkEngineDemo PRIVATE
    SharkEngine
)

if(EMSCRIPTEN)
    # Shaders are preloaded into the generated .data file.  Make the link step
    # depend on them so a shader-only edit refreshes the browser artifact.
    set_property(TARGET SharkEngineDemo APPEND PROPERTY LINK_DEPENDS
        "${SharkEngine_SOURCE_DIR}/engine/res/shaders/default.vert.glsl"
        "${SharkEngine_SOURCE_DIR}/engine/res/shaders/default.frag.glsl"
        "${SharkEngine_SOURCE_DIR}/engine/res/shaders/raytrace.vert.glsl"
        "${SharkEngine_SOURCE_DIR}/engine/res/shaders/raytrace.frag.glsl"
    )

    # Output SharkEngineDemo.html, SharkEngineDemo.js, and SharkEngineDemo.wasm
    set_target_properties(SharkEngineDemo PROPERTIES SUFFIX ".html")

    target_link_options(SharkEngineDemo PRIVATE
        "-sUSE_GLFW=3"
        "-sFULL_ES3=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "--preload-file" "${SharkEngine_SOURCE_DIR}/engine/res@/res"
        "--shell-file" "${CMAKE_CURRENT_SOURCE_DIR}/web/index.html"
    )

else()
    add_custom_target(CopyEngineResources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SharkEngine_SOURCE_DIR}/engine/res"
            "$<TARGET_FILE_DIR:SharkEngineDemo>/res"
        COMMENT "Copying engine resources to output directory"
    )
    add_dependencies(SharkEngineDemo CopyEngineResources)

    # Enable app-local dependency copying for vcpkg on Windows
    if(WIN32)
        set(VCPKG_APPLOCAL_DEPS ON)
    endif()

    install(TARGETS SharkEngineDemo RUNTIME DESTINATION bin)
    install(DIRECTORY "${SharkEngine_SOURCE_DIR}/engine/res/" DESTINATION bin/res)
endif()
