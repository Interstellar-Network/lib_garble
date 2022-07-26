################################################################################

add_subdirectory(justgarble)
add_subdirectory(evaluate)
add_subdirectory(packmsg)
add_subdirectory(parallel_garbled_circuit)
add_subdirectory(serialize_packmsg)
add_subdirectory(serialize_pgc)
add_subdirectory(watermark)

################################################################################
# PUBLIC API

add_library(garble_helper
    garble_helper.cpp
)

target_link_libraries(garble_helper
    PUBLIC
    # needed b/c included in garble_helper.h
    pgc_lib

    PRIVATE
    glog::glog
    absl::strings
)

target_include_directories(garble_helper
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)


################################################################################
# PUBLIC API

add_library(packmsg_helper
    packmsg_helper.cpp
)

target_include_directories(packmsg_helper
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(packmsg_helper
    PUBLIC
    pre_packmsg
    pgc_lib

    PRIVATE
    glog::glog
    absl::strings

    packmsg
    garble_helper
    packmsg_utils
    watermark_lib
)