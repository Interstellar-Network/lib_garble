cmake_minimum_required(VERSION 3.22)

project(interstellar_garble)

# ###############################################################################
include(CMakeDependentOption)

option(interstellar_lib_garble_BUILD_TESTS "Build tests" ON)
cmake_dependent_option(interstellar_lib_garble_HAS_CLIS "Build the various testing cli" ON "interstellar_lib_garble_BUILD_TESTS;" ON)

# ###############################################################################
include(${CMAKE_CURRENT_LIST_DIR}/deps/cmake/compile_flags.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/cmake/coverage.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/cmake/options.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/cmake/sanitizers.cmake)

# ###############################################################################
include(${CMAKE_CURRENT_LIST_DIR}/3rd_party/abseil.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/3rd_party/glog.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/3rd_party/protobuf.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/3rd_party/utfcpp.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/deps/protos.cmake)

# ###############################################################################

# the main sources
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/data)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src)

if(interstellar_lib_garble_BUILD_TESTS)
  enable_testing()
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tests)
endif(interstellar_lib_garble_BUILD_TESTS)