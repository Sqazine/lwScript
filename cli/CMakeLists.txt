
add_executable(lwScript lwScript.cpp)
target_link_libraries(lwScript PRIVATE liblwScript)
target_include_directories(lwScript PRIVATE ${CMAKE_SOURCE_DIR}/liblwScript)
