file(GLOB INC "${CMAKE_SOURCE_DIR}/liblwScript/inc/*.h")
file(GLOB SRC "${CMAKE_SOURCE_DIR}/liblwScript/src/*.cpp")

source_group("liblwScript/inc" FILES ${INC})
source_group("liblwScript/src" FILES ${SRC})

add_library(${LIB_NAME} SHARED ${SRC} ${INC})

target_compile_definitions(${LIB_NAME} PUBLIC LWSCRIPT_BUILD_DLL)
target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)