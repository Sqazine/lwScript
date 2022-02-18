cmake_minimum_required(VERSION 3.10)

project(lwScript LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSION OFF)

add_subdirectory(lwScript)

add_executable(lwScript lwScript.cpp)
target_link_libraries(lwScript liblwScript)
