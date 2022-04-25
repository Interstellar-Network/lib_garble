include(${PROJECT_SOURCE_DIR}/3rd_party/_conan.cmake)

################################################################################

include(${CMAKE_BINARY_DIR}/conan.cmake)


conan_cmake_configure(REQUIRES glog/0.6.0
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
find_package(glog REQUIRED)

return()

################################################################################

include(FetchContent)

# only way to disable glog tests...
# https://github.com/google/glog/pull/200
# "clean workaround" apparently
set(BUILD_TESTING_SAVED "${BUILD_TESTING}")
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glog
    # v0.5.0 released this May 08, 2021
    # but it does NOT compile with clang 13 and all the warnings
    GIT_REPOSITORY  https://github.com/google/glog.git
    # Commits on Feb 20, 2022
    GIT_TAG     a8e0007e96ff96145022c488e36
)

FetchContent_MakeAvailable(glog)

set(BUILD_TESTING "${BUILD_TESTING_SAVED}" CACHE BOOL "" FORCE)