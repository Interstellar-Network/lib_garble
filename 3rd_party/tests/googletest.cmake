# ##############################################################################
# conan with "BUILD all" recompile both openssl and CMake from source??
if(USE_CONAN)
  include(${CMAKE_CURRENT_LIST_DIR}/../_conan.cmake)

  include(${CMAKE_BINARY_DIR}/conan.cmake)

  conan_cmake_configure(REQUIRES gtest/1.12.1
    GENERATORS cmake_find_package)

  # NO!
  # FAIL:
  # ERROR: Missing prebuilt package for...
  # - We DO NOT care if the package was built with gcc even if locally we are using clang
  # - We WANT to always use Release libs even when building Debug locally(SHOULD be configurable)
  # --> only works reliably with "BUILD all" cf below for details
  conan_cmake_autodetect(settings)
  message(STATUS "conan settings : ${settings}")

  conan_cmake_install(PATH_OR_REFERENCE .
    REMOTE conancenter

    # IMPORTANT, b/c it ends up impacting ABSL_OPTION_USE_STD_STRING_VIEW
    # which is 0 without this, which means it uses absl internal string_view
    # which means there is NO conversion from str::string_view -> absl::string_view
    # and that breaks a lot of function
    # MUST AT LEAST set # SETTINGS compiler.cppstd=${CMAKE_CXX_STANDARD}
    SETTINGS ${settings}

    # IMPORTANT: we MUST make sure we have repeatable builds, DO NOT use "missing"
    # else we get random "Exception: Illegal" during tests in CI
    BUILD all
  )

  # cf build/tests/FindGTest.cmake for the vars
  find_package(GTest REQUIRED)

  include(GoogleTest)

  return()
endif(USE_CONAN)

# ###############################################################################
include(FetchContent)
FetchContent_Declare(
  googletest

  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)

# "For Windows: Prevent overriding the parent project's compiler/linker settings"
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)