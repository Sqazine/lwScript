cmake_minimum_required(VERSION 3.10)

project(lwScript LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX "_d") 
set(CMAKE_RELEASE_POSTFIX "")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(STATUS "current platform: Linux")
    set(CURRENT_PLATFORM PLATFORM_LINUX)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    message(STATUS "current platform: Windows")
    set(CURRENT_PLATFORM PLATFORM_WINDOWS)
elseif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    message(STATUS "current platform: FreeBSD")
    set(CURRENT_PLATFORM PLATFORM_FREEBSD)
else ()
    message(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
    set(CURRENT_PLATFORM PLATFORM_OTHER)
endif ()

message(STATUS "CURRENT_PLATFORM=${CURRENT_PLATFORM}")

# set(CMAKE_CXX_FLAGS -w)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(liblwscript)

file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.lws")

source_group("examples" FILES ${EXAMPLES})
add_executable(lwScript lwScript.cpp ${EXAMPLES})
set_target_properties(lwScript PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

if(${CURRENT_PLATFORM} STREQUAL "PLATFORM_WINDOWS")
    target_link_libraries(lwScript PRIVATE liblwScript)
else()
    target_link_libraries(lwScript PRIVATE lwScript)
endif()

