include(${CMAKE_CURRENT_LIST_DIR}/../_conan.cmake)

################################################################################

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_configure(REQUIRES gtest/1.11.0
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
                    # same kind of issues than abseil(absl)
                    # error: undefined symbol: testing::Matcher<std::basic_string_view<char, std::char_traits<char> > const&>::Matcher(char const*)git >>> referenced by segments2pixels_test.cpp
                    SETTINGS compiler.cppstd=${CMAKE_CXX_STANDARD}
                    # IMPORTANT: we MUST make sure we have repeatable builds, DO NOT use "missing"
                    # else we get random "Exception: Illegal" during tests in CI
                    BUILD all
)

# cf build/tests/FindGTest.cmake for the vars
find_package(GTest REQUIRED)

include(GoogleTest)

return()

################################################################################

include(FetchContent)
FetchContent_Declare(
  googletest
  # Commits on Feb 14, 2022
  URL https://github.com/google/googletest/archive/ea55f1f52c489535f0d3b583c81529762c9cb5ea.zip
)

# "For Windows: Prevent overriding the parent project's compiler/linker settings"
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)