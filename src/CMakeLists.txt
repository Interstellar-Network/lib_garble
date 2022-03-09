################################################################################

add_subdirectory(justgarble)
add_subdirectory(evaluate)
add_subdirectory(parallel_garbled_circuit)
add_subdirectory(serialize)

################################################################################

add_library(garble_helper
    garble_helper.cpp
)

target_link_libraries(garble_helper
    PRIVATE
    glog
    absl::strings

    serialize_lib

    PUBLIC
    # public that way linking with utils_file and #include "utils_file.h" will "just work"
    ${Boost_FILESYSTEM_LIBRARY}
)

target_include_directories(garble_helper
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
