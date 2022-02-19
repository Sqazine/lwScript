file(GLOB SRC "*.h" "*.cpp")

source_group("liblwScript" FILES ${SRC})

add_library(liblwScript ${SRC})