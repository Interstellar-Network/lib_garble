################################################################################

add_library(pre_packmsg
    prepackmsg.cpp
)

target_include_directories(pre_packmsg
    INTERFACE
    # that way users will need to #include "evaluate/evaluate.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(pre_packmsg
    PUBLIC
    # used in .h so PUBLIC
    absl::flat_hash_map

    pgc_lib

    PRIVATE
    glog::glog
)

################################################################################

add_library(packmsg_utils
    packmsg_utils.cpp
)

target_include_directories(packmsg_utils
    INTERFACE
    # that way users will need to #include "evaluate/evaluate.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(packmsg_utils
    PUBLIC
    pgc_lib

    PRIVATE
    glog::glog
    watermark_lib
)

################################################################################

add_library(packmsg
    packmsg.cpp
    stripped_circuit.cpp
)

target_include_directories(packmsg
    INTERFACE
    # that way users will need to #include "evaluate/evaluate.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(packmsg
    PUBLIC
    # used in .h so PUBLIC
    absl::random_random

    pre_packmsg

    PRIVATE
    glog::glog
    packmsg_utils
)