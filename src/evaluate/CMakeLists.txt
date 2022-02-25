# TODO move to separate repo

# flags are only needed for GCC
# with Clang : "Could NOT find OpenMP_C (missing: OpenMP_C_FLAGS OpenMP_C_LIB_NAMES)"
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")

find_package(OpenMP REQUIRED)

endif()

################################################################################

add_library(evaluate_lib
    evaluate.cpp
)

target_include_directories(evaluate_lib
    INTERFACE
    # that way users will need to #include "evaluate/evaluate.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(evaluate_lib
    # PUBLIC b/c needed by the .h
    PUBLIC
    justgarble_custom

    PRIVATE
    # flags are only needed for GCC
    $<$<CXX_COMPILER_ID:GNU>:OpenMP::OpenMP_CXX>
)