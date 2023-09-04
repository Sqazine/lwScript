cmake_minimum_required(VERSION 3.10)

project(lwScript LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSION OFF)

# set(CMAKE_CXX_FLAGS -w)

file(GLOB SRC "liblwScript/*.h" "liblwScript/*.cpp")
source_group("liblwScript" FILES ${SRC})
add_library(liblwScript ${SRC})
add_executable(lwScript lwScript.cpp)
target_link_libraries(lwScript PRIVATE liblwScript)