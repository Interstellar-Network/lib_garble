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

target_compile_options(libprotobuf
  PRIVATE
  -Wno-sign-compare
  -Wno-unused-parameter
  -Wno-invalid-noreturn
)