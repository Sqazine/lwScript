cmake_minimum_required(VERSION 3.10)

project(lwScript LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSION OFF)

file(GLOB LWSCRIPT_SRC "lwScript/*.h" "lwScript/*.cpp")
add_library(liblwScript STATIC ${LWSCRIPT_SRC})


add_executable(lwScript lwScript.cpp )
target_link_libraries(lwScript PRIVATE liblwScript)
