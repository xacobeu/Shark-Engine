set(_probe "${CMAKE_BINARY_DIR}/shark_std_probe.cpp")
file(WRITE "${_probe}" "#include <version>\n")
separate_arguments(_flags NATIVE_COMMAND "${CMAKE_CXX_FLAGS}")
execute_process(
    COMMAND "${CMAKE_CXX_COMPILER}" ${_flags} -std=gnu++26 -fsyntax-only -H "${_probe}"
    ERROR_VARIABLE _headers
    OUTPUT_QUIET
)
string(REPLACE "\\\\" "/" _headers "${_headers}")
string(REPLACE "\\" "/" _headers "${_headers}")
string(REGEX MATCH "\\. ([^\n]*)/version\n" _ "${_headers}")
set(_include_dir "${CMAKE_MATCH_1}")

foreach(_candidate
    "${_include_dir}/../modules/std.ixx"
    "${_include_dir}/bits/std.cc"
    "${_include_dir}/../../../share/libc++/v1/std.cppm"
    "${EMSCRIPTEN_ROOT_PATH}/system/lib/libcxx/modules/prebuilt/share/libc++/v1/std.cppm"
)
    if(_include_dir AND EXISTS "${_candidate}" AND NOT IS_DIRECTORY "${_candidate}")
        get_filename_component(SHARK_STD_MODULE_SOURCE "${_candidate}" REALPATH)
        break()
    endif()
endforeach()

if(NOT SHARK_STD_MODULE_SOURCE)
    message(FATAL_ERROR
        "No `std` module source found next to the standard library headers in '${_include_dir}'.\n"
        "SharkEngine needs MSVC STL (VS 17.6+), libstdc++ from GCC 15+, or libc++ with modules installed.")
endif()
message(STATUS "std module source: ${SHARK_STD_MODULE_SOURCE}")

get_filename_component(_std_dir "${SHARK_STD_MODULE_SOURCE}" DIRECTORY)
set_source_files_properties("${SHARK_STD_MODULE_SOURCE}" PROPERTIES LANGUAGE CXX)

add_library(shark_std STATIC)
target_sources(shark_std PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${_std_dir}" FILES "${SHARK_STD_MODULE_SOURCE}")
target_compile_features(shark_std PUBLIC cxx_std_26)
target_compile_options(shark_std PRIVATE -w)
