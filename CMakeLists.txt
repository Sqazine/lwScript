cmake_minimum_required(VERSION 3.10)

project(lwScript LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSION OFF)

# set(CMAKE_CXX_FLAGS -w)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Bin/Release)

file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.lws")

file(GLOB SRC "liblwScript/*.h" "liblwScript/*.cpp")
source_group("liblwScript" FILES ${SRC})
add_library(lwScript SHARED ${SRC})
target_compile_definitions(lwScript PUBLIC LWSCRIPT_BUILD_DLL)

add_executable(lwScript-cli lwScript.cpp ${EXAMPLES})
target_link_libraries(lwScript-cli PRIVATE lwScript)