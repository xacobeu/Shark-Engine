file(GLOB_RECURSE EXAMPLE_MODULES CONFIGURE_DEPENDS "src/*.cppm")
list(FILTER EXAMPLE_MODULES EXCLUDE REGEX "/Main\\.cppm$")
set_source_files_properties(src/Main.cppm PROPERTIES LANGUAGE CXX COMPILE_OPTIONS "-xc++")
add_executable(SharkEngineExample src/Main.cppm)
target_sources(SharkEngineExample PRIVATE FILE_SET CXX_MODULES BASE_DIRS src FILES ${EXAMPLE_MODULES})

target_link_libraries(SharkEngineExample PRIVATE
    SharkEngine
)

if(EMSCRIPTEN)
    target_link_options(SharkEngineExample PRIVATE
        "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/res@/res"
    )
else()
    add_custom_command(TARGET SharkEngineExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:SharkEngineExample>/res"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:SharkEngineExample>/res"
        COMMENT "Copying engine and example resources to output directory"
    )
endif()

