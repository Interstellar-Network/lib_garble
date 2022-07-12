include(${CMAKE_CURRENT_LIST_DIR}/_conan.cmake)

################################################################################

include(${CMAKE_BINARY_DIR}/conan.cmake)


conan_cmake_configure(REQUIRES abseil/20211102.0
                      GENERATORS cmake_find_package)

# NO!
# FAIL:
# ERROR: Missing prebuilt package for...
# - We DO NOT care if the package was built with gcc even if locally we are using clang
# - We WANT to always use Release libs even when building Debug locally(SHOULD be configurable)
# conan_cmake_autodetect(settings)

message(WARNING "settings : ${settings}")

conan_cmake_install(PATH_OR_REFERENCE .
                    # NO! we WANT the prebuilt binary
                    # BUILD missing
                    REMOTE conancenter
                    # SETTINGS ${settings}
)

# cf build/Findabsl.cmake for the vars
find_package(absl REQUIRED)

return()

################################################################################

include(FetchContent)

FetchContent_Declare(
    abseil
    # "Abseil recommends users "live-at-head" (update to the latest commit from the master branch as often as possible)."
    # Commits on Feb 17, 2022
    GIT_REPOSITORY  https://github.com/abseil/abseil-cpp.git
    GIT_TAG     7f850b3167fb38e6b4a9ce1824e6fabd733b5d62
)

# avoids a warning:
# A future Abseil release will default ABSL_PROPAGATE_CXX_STD to ON for CMake
# [build]   3.8 and up.  We recommend enabling this option to ensure your project still
# [build]   builds correctly.
option(ABSL_PROPAGATE_CXX_STD
  "Use CMake C++ standard meta features (e.g. cxx_std_11) that propagate to targets that link to Abseil"
  ON) # default to OFF

FetchContent_MakeAvailable(abseil)
