# ###############################################################################

include(${PROJECT_SOURCE_DIR}/3rd_party/tests/cimg.cmake)
include(${PROJECT_SOURCE_DIR}/3rd_party/tests/googletest.cmake)

# NOTE: enable_testing() SHOULD be called from the root according to the docs

# ###############################################################################
add_subdirectory(data)
add_subdirectory(utils)

# ###############################################################################

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
        GTest::gtest
        GTest::gtest_main
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
add_google_test(test_garble "garble_helper serialize_pgc_lib GTest::gmock testing_utils_files test_data")
gtest_discover_tests(test_garble)
add_google_test(test_garble_reference "justgarble_custom pgc_lib GTest::gmock")
gtest_discover_tests(test_garble_reference)
add_google_test(test_evaluate_stripped "serialize_pgc_lib serialize_packmsg_lib testing_utils_files packmsg_helper justgarble_custom evaluate_lib test_data GTest::gmock absl::strings")
gtest_discover_tests(test_evaluate_stripped)
add_google_test(test_evaluate "serialize_pgc_lib packmsg_helper justgarble_custom evaluate_lib test_data GTest::gmock")
gtest_discover_tests(test_evaluate)
add_google_test(test_serialization "serialize_pgc_lib test_data GTest::gmock")
gtest_discover_tests(test_serialization)
add_google_test(test_stripped "packmsg_helper serialize_packmsg_lib testing_utils_files test_data GTest::gmock")
gtest_discover_tests(test_stripped)

# ###############################################################################
# NOTE: only for some eval cli(required by cimg.save_png()
if(interstellar_lib_garble_HAS_CLIS)
    find_package(PNG REQUIRED)

    # we need b/c we are dynamically listing the required libs for the Rust wrapper
    # and without this we would be missing the system libs(eg boost, libpng, etc)
    # ie all those obtained using find_package b/c IMPORTED are LOCAL by default
    # cf https://gitlab.kitware.com/cmake/cmake/-/issues/17256
    # MAYBE ALIAS_GLOBAL?
    set_target_properties(PNG::PNG PROPERTIES IMPORTED_GLOBAL TRUE)

    # ###############################################################################
    add_library(utils_eval
        utils_eval.cpp
    )

    target_link_libraries(utils_eval
        PUBLIC

        PRIVATE
        evaluate_lib
        pgc_lib
        packmsg

        cimg
        X11
        PNG::PNG
    )

    # basic cli(NOT googletest); but that still works fine
    add_google_test(cli_garble "garble_helper serialize_pgc_lib testing_utils_files absl::flags absl::flags_parse")
    add_google_test(cli_garble_and_eval_stripped "garble_helper serialize_packmsg_lib serialize_pgc_lib packmsg_helper utils_eval testing_utils_files absl::flags absl::flags_parse")
    add_google_test(cli_garble_and_eval "garble_helper serialize_pgc_lib utils_eval testing_utils_files absl::flags absl::flags_parse")
    add_google_test(cli_eval "evaluate_lib utils_eval serialize_pgc_lib absl::flags absl::flags_parse")
    add_google_test(cli_eval_stripped "evaluate_lib utils_eval serialize_pgc_lib serialize_packmsg_lib testing_utils_files absl::flags absl::flags_parse")
endif(interstellar_lib_garble_HAS_CLIS)

# ###############################################################################
