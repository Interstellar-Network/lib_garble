################################################################################
# prerequisite : Boost: filesystem
find_package(Boost COMPONENTS filesystem REQUIRED)

################################################################################

add_library(justgarble_custom
    garble_helper.cpp
    garble_utils.cpp
    garble.cpp
    parallel.cpp
    utils_block.cpp
    xor_gate_processor.cpp
    xor_graph.cpp
)

target_link_libraries(justgarble_custom
    PRIVATE
    glog
    absl::strings

    interstellar_protos
    serialize_lib

    PUBLIC
    # public that way linking with utils_file and #include "utils_file.h" will "just work"
    ${Boost_FILESYSTEM_LIBRARY}
)

target_include_directories(justgarble_custom
    PUBLIC
    # that way users will need to #include "justgarble/garble.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(justgarble_custom
    # TODO should this be public? probabably yes b/c filesystem.hpp is included by the internal .h
    PUBLIC BOOST_FILESYSTEM_NO_DEPRECATED
)
