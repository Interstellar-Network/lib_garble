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