cmake_minimum_required(VERSION 3.10)

project(lwScript LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX "_d") 
set(CMAKE_RELEASE_POSTFIX "")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CURRENT_PLATFORM PLATFORM_LINUX)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CURRENT_PLATFORM PLATFORM_WINDOWS)
elseif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    set(CURRENT_PLATFORM PLATFORM_FREEBSD)
else ()
    set(CURRENT_PLATFORM PLATFORM_OTHER)
endif ()

message(STATUS "CURRENT_PLATFORM=${CURRENT_PLATFORM}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

set(EXE_NAME lwScript)

if(MSVC)
    set(LIB_NAME liblwScript)
else()
    set(LIB_NAME lwScript)
endif()

add_subdirectory(liblwscript)

file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.lws")

source_group("examples" FILES ${EXAMPLES})
add_executable(lwScript lwScript.cpp ${EXAMPLES})
set_target_properties(lwScript PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_include_directories(lwScript PUBLIC ${CMAKE_SOURCE_DIR}/liblwscript/inc)
target_link_libraries(lwScript PRIVATE ${LIB_NAME})

if(MSVC)
    target_compile_definitions(${LIB_NAME} PUBLIC NOMINMAX)
    target_compile_definitions(${EXE_NAME} PUBLIC NOMINMAX)

    target_compile_definitions(${LIB_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(${EXE_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)

    target_compile_options(${LIB_NAME} PRIVATE "/wd4251;")
    target_compile_options(${EXE_NAME} PRIVATE "/wd4251;")
    set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME} )
endif()