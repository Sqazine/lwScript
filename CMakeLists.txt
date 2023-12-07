cmake_minimum_required(VERSION 3.10)

project(lwScript LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX "_d") 
set(CMAKE_RELEASE_POSTFIX "")

# set(CMAKE_CXX_FLAGS -w)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(liblwscript)

file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.lws")

add_executable(lwScript-cli lwScript.cpp ${EXAMPLES})
target_link_libraries(lwScript-cli PRIVATE lwScript)