include(FetchContent)
FetchContent_Declare(
  protobuf_fetch
  GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
  GIT_TAG v3.19.4
  SOURCE_SUBDIR cmake
)

option(protobuf_INSTALL "Install protobuf binaries and files" OFF) # default ON
option(protobuf_BUILD_TESTS "Build tests" OFF) # default ON
option(protobuf_BUILD_CONFORMANCE "Build conformance tests" OFF)
option(protobuf_BUILD_EXAMPLES "Build examples" OFF)
option(protobuf_BUILD_PROTOC_BINARIES "Build libprotoc and protoc compiler" ON)
option(protobuf_BUILD_LIBPROTOC "Build libprotoc" OFF)
option(protobuf_DISABLE_RTTI "Remove runtime type information in the binaries" OFF)
option(protobuf_WITH_ZLIB "Build with zlib support" OFF)  # default ON

FetchContent_MakeAvailable(protobuf_fetch)

# NOTE: we could be a bit more granular but:
# - those are PRIVATE -Wno-xxx so not that big a deal for our own code quality
# - we can trust Google's standards and CI
set(PROTO_CXX_FLAGS
  -Wno-sign-compare
  -Wno-redundant-move
  -Wno-unused-parameter
  $<$<CXX_COMPILER_ID:Clang>:
    -Wno-invalid-noreturn
  >
  $<$<CXX_COMPILER_ID:GNU>:
    -Wno-stringop-overflow
    # no way to disable it...
    # error: ‘noreturn’ function does return [-Werror]
    # Tried "set_source_files_properties" but no success
    -Wno-error
  >
)

target_compile_options(libprotobuf-lite
  PRIVATE
  ${PROTO_CXX_FLAGS}
)
# libprotobuf b/c
#
# add_library(libprotobuf ${protobuf_SHARED_OR_STATIC}
# ${libprotobuf_lite_files} ${libprotobuf_files} ${libprotobuf_includes} ${libprotobuf_rc_files})
# ...
target_compile_options(libprotobuf
  PRIVATE
  ${PROTO_CXX_FLAGS}
)
target_compile_options(libprotoc
  PRIVATE
  ${PROTO_CXX_FLAGS}
)
target_compile_options(protoc
  PRIVATE
  ${PROTO_CXX_FLAGS}
)

# eg protobuf_fetch_SOURCE_DIR =  /.../lib_circuits/build/_deps/protobuf_fetch-src
# message(FATAL_ERROR "protobuf_fetch : ${protobuf_fetch_SOURCE_DIR}")

# error: ‘noreturn’ function does return [-Werror]
# at src/google/protobuf/generated_message_tctable_impl.h:10
# set_source_files_properties("${protobuf_fetch_SOURCE_DIR}/src/google/protobuf/generated_message_tctable_impl.h" DIRECTORY "${PROJECT_SOURCE_DIR}" PROPERTIES COMPILE_OPTIONS "-Wno-error;")
# set_source_files_properties("${protobuf_fetch_SOURCE_DIR}/src/google/protobuf/generated_message_tctable_lite.cc" DIRECTORY "${PROJECT_SOURCE_DIR}" PROPERTIES COMPILE_OPTIONS "-Wno-error;")