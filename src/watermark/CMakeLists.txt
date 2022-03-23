################################################################################

find_package(Freetype REQUIRED)

################################################################################

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

    resources
)