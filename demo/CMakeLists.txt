add_executable(SharkEngineDemo
    src/Main.cpp
    src/layers/Example.cpp
)

target_compile_options(SharkEngineDemo PRIVATE ${SHARK_COMPILER_OPTIONS})

target_link_libraries(SharkEngineDemo PRIVATE
    SharkEngine
)

# Ensure C++20 features are available to this target
target_compile_features(SharkEngineDemo PRIVATE cxx_std_20)

# Copy required DLLs to output directory on Windows
if(WIN32)
    add_custom_command(TARGET SharkEngineDemo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/bin/glfw3.dll"
        $<TARGET_FILE_DIR:SharkEngineDemo>
    )
endif()

# Copy res folder
add_custom_command(TARGET SharkEngineDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/res"
        "$<TARGET_FILE_DIR:SharkEngineDemo>/res"
)

# Copy shaders folder
add_custom_command(TARGET SharkEngineDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/res/shaders"
        "$<TARGET_FILE_DIR:SharkEngineDemo>/res/shaders"
)

# Install the executable and resources
install(TARGETS SharkEngineDemo RUNTIME DESTINATION bin)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/res/" DESTINATION bin/res)

# Automatically copy necessary DLLs (Windows specific)
if(WIN32)
    set(VCPKG_BIN_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/bin")
    file(GLOB VCPKG_DLLS "${VCPKG_BIN_DIR}/*.dll")
    install(FILES ${VCPKG_DLLS} DESTINATION bin)
endif()
