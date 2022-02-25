################################################################################

add_library(serialize_lib
    serialize.cpp
)

target_include_directories(serialize_lib
    INTERFACE
    # that way users will need to #include "evaluate/evaluate.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(serialize_lib
    # PUBLIC b/c needed by the .h
    PUBLIC
    justgarble_custom

    PRIVATE
    glog::glog
    interstellar_protos
)