# Attempt to fix the deprecation warning
if(EXISTS "${LUA_SRC_DIR}/CMakeLists.txt")
    file(READ "${LUA_SRC_DIR}/CMakeLists.txt" content)
    string(REPLACE "cmake_minimum_required(VERSION 3.1)" "cmake_minimum_required(VERSION 3.10)" content "${content}")
    file(WRITE "${LUA_SRC_DIR}/CMakeLists.txt" "${content}")
    message(STATUS "Patched ${LUA_SRC_DIR}/CMakeLists.txt cmake_minimum_required version to 3.10")
endif()
