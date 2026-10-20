file(GLOB_RECURSE EXAMPLE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(SharkEngineExample ${EXAMPLE_SOURCES})

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

