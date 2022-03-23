################################################################################
# PUBLIC API

add_library(serialize_pgc_lib
    serialize.cpp
)

target_include_directories(serialize_pgc_lib
    INTERFACE
    # that way users will need to #include "evaluate/evaluate.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(serialize_pgc_lib
    # PUBLIC b/c needed by the .h
    PUBLIC
    pgc_lib

    PRIVATE
    glog::glog
    interstellar_protos
)