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

set(LWS_DEBUG_POSTFIX "_d" CACHE STRING "library or executable debug version postfix")
set(LWS_RELEASE_POSTFIX "" CACHE STRING "library or executable release version postfix")
option(LWS_STATIC_BUILD "build lwScript library as static library" OFF)
option(LWS_BUILD_EXECUTABLE "build lwScript executable file" ON)
option(LWS_UTF8_ENCODE "use utf8 encode" ON)
option(LWS_FUNCTION_CACHE_OPT "use runtime optimize feature:function cache" ON)
option(LWS_CONSTANT_FOLD_OPT "use ast optimize feature:constant fold" ON)
option(LWS_GC_DEBUG "output gc debug information" OFF)
option(LWS_GC_STRESS "force call gc after creating object in runtime" OFF)

set(CMAKE_DEBUG_POSTFIX ${LWS_DEBUG_POSTFIX}) 
set(CMAKE_RELEASE_POSTFIX ${LWS_RELEASE_POSTFIX})

set(LWS_BINARY_DIR ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LWS_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LWS_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${LWS_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${LWS_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LWS_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LWS_BINARY_DIR})


if(MSVC OR ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    set(LIB_NAME liblwScript)
else()
    set(LIB_NAME lwScript)
endif()

file(GLOB LWS_HEADER_FILES "${GENERATED_DIR}/*.h" "*.h")
file(GLOB LWS_SOURCE_FILES "${GENERATED_DIR}/*.cpp" "*.cpp")

source_group("src" FILES ${LWS_HEADER_FILES} ${LWS_SOURCE_FILES})
if(LWS_STATIC_BUILD)
    add_library(${LIB_NAME} STATIC ${LWS_HEADER_FILES} ${LWS_SOURCE_FILES})
    target_compile_definitions(${LIB_NAME} PUBLIC LWS_BUILD_STATIC)
else()
    add_library(${LIB_NAME} SHARED ${LWS_HEADER_FILES} ${LWS_SOURCE_FILES})
    target_compile_definitions(${LIB_NAME} PUBLIC LWS_BUILD_DLL)
endif()
target_include_directories(${LIB_NAME} PRIVATE ${GENERATED_DIR})

if(LWS_BUILD_EXECUTABLE)
    set(EXE_NAME lwscript)

    file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.lws")

    source_group("examples" FILES ${EXAMPLES})
    add_executable(${EXE_NAME} lwScript.cc ${EXAMPLES})
    set_target_properties(${EXE_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_include_directories(${EXE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/liblwScript)
    target_include_directories(${EXE_NAME} PRIVATE ${GENERATED_DIR})
    target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})
endif()

if(LWS_UTF8_ENCODE)
    target_compile_definitions(${LIB_NAME} PUBLIC LWS_UTF8_ENCODE)
    if(LWS_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC LWS_UTF8_ENCODE)
    endif()
endif()

if(LWS_GC_DEBUG)
    target_compile_definitions(${LIB_NAME} PUBLIC LWS_GC_DEBUG)
    if(LWS_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC LWS_GC_DEBUG)
    endif()
endif()

if(LWS_GC_STRESS)
    target_compile_definitions(${LIB_NAME} PUBLIC LWS_GC_STRESS)
    if(LWS_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC LWS_GC_STRESS)
    endif()
endif()

if(LWS_FUNCTION_CACHE_OPT)
    target_compile_definitions(${LIB_NAME} PUBLIC LWS_FUNCTION_CACHE_OPT)
    if(LWS_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC LWS_FUNCTION_CACHE_OPT)
    endif()
endif()

if(LWS_CONSTANT_FOLD_OPT)
    target_compile_definitions(${LIB_NAME} PUBLIC LWS_CONSTANT_FOLD_OPT)
    if(LWS_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC LWS_CONSTANT_FOLD_OPT)
    endif()
endif()

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    target_compile_definitions(${LIB_NAME} PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    if(LWS_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    endif()

    if(MSVC)
        target_compile_options(${LIB_NAME} PRIVATE "/wd4251;" "/bigobj;" "/MD;")
        if(LWS_BUILD_EXECUTABLE)
            target_compile_options(${EXE_NAME} PRIVATE "/wd4251;" "/bigobj;" "/MD;")
            set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME})
        endif()
    endif()
else()
    target_compile_options(${LIB_NAME} PRIVATE "-fpermissive")
    if(LWS_BUILD_EXECUTABLE)
        target_compile_options(${EXE_NAME} PRIVATE "-fpermissive")
    endif()
endif()

file(COPY ${LWS_HEADER_FILES} DESTINATION ${LWS_BINARY_DIR}/inc)