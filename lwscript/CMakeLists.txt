file(GLOB SRC "*.h" "*.cpp")

source_group("lwScript" ${SRC})

add_library(liblwScript ${SRC})