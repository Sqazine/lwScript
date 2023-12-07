file(GLOB INC "${CMAKE_SOURCE_DIR}/liblwScript/inc/*.h")
file(GLOB SRC "${CMAKE_SOURCE_DIR}/liblwScript/src/*.cpp")

source_group("liblwScript/inc" FILES ${INC})
source_group("liblwScript/src" FILES ${SRC})

add_library(lwScript SHARED ${SRC})

target_compile_definitions(lwScript PUBLIC LWSCRIPT_BUILD_DLL)
target_include_directories(lwScript PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)