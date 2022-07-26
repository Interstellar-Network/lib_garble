# ##############################################################################
include(FetchContent)

FetchContent_Declare(
    utfcpp

    GIT_REPOSITORY https://github.com/nemtrif/utfcpp.git

    GIT_TAG v3.2.1
)

FetchContent_MakeAvailable(utfcpp)
