cmake_minimum_required(VERSION 3.10)

project(lwScript VERSION 0.1.0 LANGUAGES C CXX)

configure_file(Version.h.in generated/Version.h @ONLY)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_DEBUG_POSTFIX "_d") 
set(CMAKE_RELEASE_POSTFIX "")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)

option(USE_UTF8_ENCODE "use utf8 encode" ON)

set(EXE_NAME lwScript)

if(MSVC)
    set(LIB_NAME liblwScript)
else()
    set(LIB_NAME lwScript)
endif()

file(GLOB SRC "liblwScript/*.h" "liblwScript/*.cpp" "${GENERATED_DIR}/*.h")
source_group("src" FILES ${SRC})
add_library(${LIB_NAME} SHARED ${SRC})
target_include_directories(${LIB_NAME} PRIVATE ${GENERATED_DIR})
target_compile_definitions(${LIB_NAME} PUBLIC LWSCRIPT_BUILD_DLL)

file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.lws")

source_group("examples" FILES ${EXAMPLES})
add_executable(${EXE_NAME} lwScript.cpp ${EXAMPLES})
set_target_properties(${EXE_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_include_directories(${EXE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/liblwscript)
target_include_directories(${EXE_NAME} PRIVATE ${GENERATED_DIR})
target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})

if(USE_UTF8_ENCODE)
    target_compile_definitions(${LIB_NAME} PUBLIC USE_UTF8_ENCODE)
    target_compile_definitions(${EXE_NAME} PUBLIC USE_UTF8_ENCODE)
endif()

if(MSVC)
    target_compile_definitions(${LIB_NAME} PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    target_compile_definitions(${EXE_NAME} PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)

    target_compile_options(${LIB_NAME} PRIVATE "/wd4251;" "/bigobj;" "/MD;")
    target_compile_options(${EXE_NAME} PRIVATE "/wd4251;" "/bigobj;" "/MD;")
    set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME})
else()
    target_compile_options(${LIB_NAME} PRIVATE "-fpermissive")
    target_compile_options(${EXE_NAME} PRIVATE "-fpermissive")
endif()