
include(FetchContent)

FetchContent_Declare(
    cimg_fetch
    # v.3.0.2 on Jan 15
    GIT_REPOSITORY  https://github.com/dtschump/CImg.git
    GIT_TAG     v.3.0.2
)

# NOT a CMake project so no option etc

FetchContent_MakeAvailable(cimg_fetch)

# NOT a CMake project, add the minimal lib manually
add_library(cimg INTERFACE)

target_include_directories(cimg
    INTERFACE
    ${cimg_fetch_SOURCE_DIR}
)