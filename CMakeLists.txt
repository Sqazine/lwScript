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

set(LWSCRIPT_DEBUG_POSTFIX "_d" CACHE STRING "library or executable debug version postfix")
set(LWSCRIPT_RELEASE_POSTFIX "" CACHE STRING "library or executable release version postfix")
option(LWSCRIPT_STATIC_BUILD "build lwscript library as static library" OFF)
option(LWSCRIPT_BUILD_EXECUTABLE "build lwscript executable file" ON)
option(LWSCRIPT_UTF8_ENCODE "use utf8 encode" ON)
option(LWSCRIPT_FUNCTION_CACHE_OPT "use runtime optimize feature:function cache" ON)
option(LWSCRIPT_CONSTANT_FOLD_OPT "use ast optimize feature:constant fold" ON)
option(LWSCRIPT_GC_DEBUG "output gc debug information" OFF)
option(LWSCRIPT_GC_STRESS "force call gc after creating object in runtime" OFF)

set(CMAKE_DEBUG_POSTFIX ${LWSCRIPT_DEBUG_POSTFIX}) 
set(CMAKE_RELEASE_POSTFIX ${LWSCRIPT_RELEASE_POSTFIX})

set(LWSCRIPT_BINARY_DIR ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${LWSCRIPT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LWSCRIPT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${LWSCRIPT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${LWSCRIPT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LWSCRIPT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LWSCRIPT_BINARY_DIR})


if(MSVC)
    set(LIB_NAME liblwScript)
else()
    set(LIB_NAME lwScript)
endif()

set(LWSCRIPT_HEADER_FILES ${GENERATED_DIR}/Version.h
                            lwscript.h
                            Allocator.h
                            Ast.h
                            AstPass.h
                            Chunk.h
                            Compiler.h
                            ConstantFoldPass.h
                            Lexer.h
                            LibraryManager.h
                            Logger.h
                            Object.h
                            Parser.h 
                            SymbolTable.h
                            SyntaxCheckPass.h 
                            Token.h
                            Type.h
                            TypeCheckPass.h
                            Utils.h
                            Value.h
                            VM.h
                            )

set(LWSCRIPT_SOURCE_FILES Allocator.cpp
                          Ast.cpp
                          Chunk.cpp
                          Compiler.cpp
                          ConstantFoldPass.cpp
                          Lexer.cpp
                          LibraryManager.cpp
                          Object.cpp
                          Parser.cpp
                          SymbolTable.cpp
                          SyntaxCheckPass.cpp
                          Type.cpp
                          TypeCheckPass.cpp
                          Utils.cpp
                          Value.cpp
                          VM.cpp
                          )

source_group("src" FILES ${LWSCRIPT_HEADER_FILES} ${LWSCRIPT_SOURCE_FILES})
if(LWSCRIPT_STATIC_BUILD)
    add_library(${LIB_NAME} STATIC ${LWSCRIPT_HEADER_FILES} ${LWSCRIPT_SOURCE_FILES})
    target_compile_definitions(${LIB_NAME} PUBLIC LWSCRIPT_BUILD_STATIC)
else()
    add_library(${LIB_NAME} SHARED ${LWSCRIPT_HEADER_FILES} ${LWSCRIPT_SOURCE_FILES})
    target_compile_definitions(${LIB_NAME} PUBLIC LWSCRIPT_BUILD_DLL)
endif()
target_include_directories(${LIB_NAME} PRIVATE ${GENERATED_DIR})

if(LWSCRIPT_BUILD_EXECUTABLE)
    set(EXE_NAME lwScript)

    file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.lws")

    source_group("examples" FILES ${EXAMPLES})
    add_executable(${EXE_NAME} lwScript.cpp ${EXAMPLES})
    set_target_properties(${EXE_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_include_directories(${EXE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/liblwscript)
    target_include_directories(${EXE_NAME} PRIVATE ${GENERATED_DIR})
    target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})
endif()

if(LWSCRIPT_UTF8_ENCODE)
    target_compile_definitions(${LIB_NAME} PUBLIC LWSCRIPT_UTF8_ENCODE)
    if(LWSCRIPT_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC LWSCRIPT_UTF8_ENCODE)
    endif()
endif()

if(LWSCRIPT_GC_DEBUG)
    target_compile_definitions(${LIB_NAME} PUBLIC LWSCRIPT_GC_DEBUG)
    if(LWSCRIPT_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC LWSCRIPT_GC_DEBUG)
    endif()
endif()

if(LWSCRIPT_GC_STRESS)
    target_compile_definitions(${LIB_NAME} PUBLIC LWSCRIPT_GC_STRESS)
    if(LWSCRIPT_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC LWSCRIPT_GC_STRESS)
    endif()
endif()

if(LWSCRIPT_FUNCTION_CACHE_OPT)
    target_compile_definitions(${LIB_NAME} PUBLIC LWSCRIPT_FUNCTION_CACHE_OPT)
    if(LWSCRIPT_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC LWSCRIPT_FUNCTION_CACHE_OPT)
    endif()
endif()

if(LWSCRIPT_CONSTANT_FOLD_OPT)
    target_compile_definitions(${LIB_NAME} PUBLIC LWSCRIPT_CONSTANT_FOLD_OPT)
    if(LWSCRIPT_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC LWSCRIPT_CONSTANT_FOLD_OPT)
    endif()
endif()

if(MSVC)
    target_compile_definitions(${LIB_NAME} PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    if(LWSCRIPT_BUILD_EXECUTABLE)
        target_compile_definitions(${EXE_NAME} PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    endif()

    target_compile_options(${LIB_NAME} PRIVATE "/wd4251;" "/bigobj;" "/MD;")
    if(LWSCRIPT_BUILD_EXECUTABLE)
        target_compile_options(${EXE_NAME} PRIVATE "/wd4251;" "/bigobj;" "/MD;")
        set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME})
    endif()
else()
    target_compile_options(${LIB_NAME} PRIVATE "-fpermissive")
    if(LWSCRIPT_BUILD_EXECUTABLE)
        target_compile_options(${EXE_NAME} PRIVATE "-fpermissive")
    endif()
endif()

file(COPY ${LWSCRIPT_HEADER_FILES} DESTINATION ${LWSCRIPT_BINARY_DIR}/inc)