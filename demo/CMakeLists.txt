add_executable(SharkEngineDemo
    src/Main.cpp
    src/layers/SpinningCubeExample.cpp
    src/layers/RayTracingExample.cpp
    src/layers/BoxExample.cpp
)

target_compile_options(SharkEngineDemo PRIVATE ${SHARK_COMPILER_OPTIONS})
target_compile_features(SharkEngineDemo PRIVATE cxx_std_20)

target_link_libraries(SharkEngineDemo PRIVATE
    SharkEngine
)

if(EMSCRIPTEN)
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
    # Copy resources to build output dir
    add_custom_command(TARGET SharkEngineDemo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SharkEngine_SOURCE_DIR}/engine/res"
            "$<TARGET_FILE_DIR:SharkEngineDemo>/res"
        COMMENT "Copying engine resources to output directory"
    )

    # Enable app-local dependency copying for vcpkg on Windows
    if(WIN32)
        set(VCPKG_APPLOCAL_DEPS ON)
    endif()

    install(TARGETS SharkEngineDemo RUNTIME DESTINATION bin)
    install(DIRECTORY "${SharkEngine_SOURCE_DIR}/engine/res/" DESTINATION bin/res)
endif()