# TODO handle installation/packaging (eg .deb and system install)
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
configure_file(resources.h.in "${CMAKE_CURRENT_BINARY_DIR}/generated/resources.h" @ONLY)

add_library(test_data INTERFACE)
target_include_directories(test_data
    INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/generated"
)