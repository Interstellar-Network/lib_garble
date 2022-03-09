################################################################################

include(${PROJECT_SOURCE_DIR}/3rd_party/cimg.cmake)
include(${PROJECT_SOURCE_DIR}/3rd_party/googletest.cmake)

# NOTE: enable_testing() SHOULD be called from the root according to the docs

################################################################################

add_subdirectory(data)

################################################################################

add_library(utils_eval
    utils_eval.cpp
)

target_link_libraries(utils_eval
    PUBLIC

    PRIVATE
    evaluate_lib
    pgc_lib

    cimg
    X11
)

################################################################################

# TODO passed list of cpp test files(eg the .cpp in tests/)
function(add_google_test test_cpp_name DEPENDENCY_LIBS)

    # space separated string -> list
    # without this target_link_libraries does not work correctly
    separate_arguments(DEPENDENCY_LIBS)

    add_executable(${test_cpp_name} ${test_cpp_name}.cpp)

    target_compile_options(${test_cpp_name} PRIVATE

    )

    target_include_directories(${test_cpp_name} PRIVATE

    )

    target_link_libraries(${test_cpp_name}
        gtest_main
        ${DEPENDENCY_LIBS}
    )

    target_compile_definitions(${test_cpp_name} PRIVATE
    )

endfunction()

# NOTE: resources is linked PRIVATE by circuit_lib(in src/);
# but relinking avoids:
# - making it public which would not be right
# - copy-pasting resources.h.in in tests/
 # NOTE: gtest_discover_tests requires cmake 3.10
add_google_test(test_garble "justgarble_custom pgc_lib gmock")
gtest_discover_tests(test_garble)
add_google_test(test_evaluate "justgarble_custom evaluate_lib test_data gmock")
gtest_discover_tests(test_evaluate)
add_google_test(test_serialization "serialize_lib test_data gmock")
gtest_discover_tests(test_serialization)

# basic cli(NOT googletest); but that still works fine
add_google_test(cli_garble "garble_helper absl::flags absl::flags_parse")
add_google_test(cli_garble_and_eval "garble_helper utils_eval absl::flags absl::flags_parse")
add_google_test(cli_eval "evaluate_lib utils_eval serialize_lib absl::flags absl::flags_parse")

################################################################################
