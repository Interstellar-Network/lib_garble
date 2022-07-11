################################################################################
# This dep is NOT using Conan b/c it CAN have MANY dependencies based on the
# enabled features, and we DO NOT need most of them
# cf https://conan.io/center/cimg?os=&tab=dependencies

include(FetchContent)

FetchContent_Declare(
    cimg_fetch
    # v.3.0.2 on Jan 15
    # NOTE: URL instead of GIT(even with GIT_SHALLOW ON)
    # real    0m5.298s
    URL  https://github.com/dtschump/CImg/archive/refs/tags/v.3.0.2.tar.gz
    # time git clone https://github.com/dtschump/CImg.git --branch v.3.0.2
    # real    0m15.698s
    # time git clone https://github.com/dtschump/CImg.git --branch v.3.0.2 --depth 1
    # real    0m4.273s
    # DEFAULT: GIT_SHALLOW OFF
    # rm -rf _deps/cimg_fetch-* && time cmake ..
    # real    0m24.099s
    # ON:
    # real    0m12.637s
    # GIT_SHALLOW ON
    # header-only so no need to do anything
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)

# NOT a CMake project so no option etc

FetchContent_MakeAvailable(cimg_fetch)

# NOT a CMake project, add the minimal lib manually
add_library(cimg INTERFACE)

target_include_directories(cimg
    INTERFACE
    ${cimg_fetch_SOURCE_DIR}
)