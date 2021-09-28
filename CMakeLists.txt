cmake_minimum_required(VERSION 3.10)

project(lwScript LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSION OFF)

file(GLOB SRC "*.h" "*.cpp")


add_executable(lwScript ${SRC})