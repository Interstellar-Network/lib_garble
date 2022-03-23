# TODO handle installation/packaging (eg .deb and system install)
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
configure_file(resources.h.in "${PROJECT_BINARY_DIR}/generated/resources.h" @ONLY)

add_library(resources INTERFACE)
target_include_directories(resources
    INTERFACE "${PROJECT_BINARY_DIR}/generated"
)