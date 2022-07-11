################################################################################
# cf https://github.com/conan-io/cmake-conan

# MUST be CMAKE_CURRENT_BINARY_DIR NOT CMAKE_BINARY_DIR else it would NOT find
# eg build/tests/FindGTest.cmake
if(NOT "${CMAKE_CURRENT_BINARY_DIR}" IN_LIST CMAKE_MODULE_PATH)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif()
if(NOT "${CMAKE_CURRENT_BINARY_DIR}" IN_LIST CMAKE_PREFIX_PATH)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif()

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
                TLS_VERIFY ON)
endif()

# NOTE: required b/c on CI we have no make, only ninja:
# "CMake Error: CMake was unable to find a build program corresponding to "Unix Makefiles""
# TODO why does this need to be set?
set(ENV{CONAN_CMAKE_GENERATOR} "${CMAKE_GENERATOR}")