################################################################################

add_library(pgc_lib
    parallel_garbled_circuit.cpp
)

target_include_directories(pgc_lib
    INTERFACE
    # that way users will need to #include "evaluate/evaluate.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(pgc_lib
    # PUBLIC b/c needed by the .h
    PUBLIC
    justgarble_custom

    PRIVATE

)