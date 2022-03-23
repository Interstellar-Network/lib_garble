################################################################################
# PUBLIC API

add_library(serialize_packmsg_lib
    serialize.cpp
)

target_include_directories(serialize_packmsg_lib
    INTERFACE
    # that way users will need to #include "evaluate/evaluate.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(serialize_packmsg_lib
    # PUBLIC b/c needed by the .h
    PUBLIC
    packmsg

    PRIVATE
    glog::glog
    interstellar_protos
)