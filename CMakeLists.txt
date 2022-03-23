cmake_minimum_required(VERSION 3.22)

project(interstellar_garble)

################################################################################

option(interstellar_garble_BUILD_TESTS "Build tests" ON)

################################################################################

include(deps/cmake/compile_flags.cmake)
include(deps/cmake/coverage.cmake)
include(deps/cmake/options.cmake)
include(deps/cmake/sanitizers.cmake)

################################################################################

include(3rd_party/abseil.cmake)
include(3rd_party/glog.cmake)
include(3rd_party/protobuf.cmake)

include(deps/protos.cmake)

################################################################################

# the main sources
add_subdirectory(data)
add_subdirectory(src)

if (interstellar_garble_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif (interstellar_garble_BUILD_TESTS)