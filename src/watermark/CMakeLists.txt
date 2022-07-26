# ###############################################################################

find_package(Freetype REQUIRED)

# we need b/c we are dynamically listing the required libs for the Rust wrapper
# and without this we would be missing the system libs(eg boost, libpng, etc)
# ie all those obtained using find_package b/c IMPORTED are LOCAL by default
# cf https://gitlab.kitware.com/cmake/cmake/-/issues/17256
# MAYBE ALIAS_GLOBAL?
set_target_properties(Freetype::Freetype PROPERTIES IMPORTED_GLOBAL TRUE)

# ###############################################################################
add_library(watermark_lib
    freetype_wrapper.cpp
    watermark.cpp
)

target_include_directories(watermark_lib
    INTERFACE

    # that way users will need to #include "evaluate/evaluate.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(watermark_lib
    PRIVATE
    glog::glog
    Freetype::Freetype
    absl::strings
    utf8::cpp

    resources
)