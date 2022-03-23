################################################################################

add_library(justgarble_custom
    garble_utils.cpp
    garble.cpp
    utils_block.cpp
    xor_gate_processor.cpp
    xor_graph.cpp
)

target_link_libraries(justgarble_custom
    PRIVATE
    glog
    absl::strings

    interstellar_protos

    PUBLIC
    absl::flat_hash_map
)

target_include_directories(justgarble_custom
    PUBLIC
    # that way users will need to #include "justgarble/garble.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
