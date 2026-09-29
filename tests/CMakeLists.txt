# --------------------------------------------------------------------
# GoogleTest
# --------------------------------------------------------------------

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

FetchContent_MakeAvailable(googletest)

# --------------------------------------------------------------------
# Test fixtures
# --------------------------------------------------------------------

# Test access fixtures relative to the working directory.
# Use a build-tree copy so tests never modify committed examples.

set(
    TEST_FIXTURES_DIR
    "${CMAKE_BINARY_DIR}/test_fixtures"
)

file(
    REMOVE_RECURSE
    "${TEST_FIXTURES_DIR}"
)

file(
    COPY 
        "${PROJECT_SOURCE_DIR}/examples/test_files"
        "${PROJECT_SOURCE_DIR}/examples/models"
    
    DESTINATION 
        "${TEST_FIXTURES_DIR}"
)

# --------------------------------------------------------------------
# Test targets
# --------------------------------------------------------------------

file(
    GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

include(GoogleTest)

foreach(test_src IN LISTS TEST_SOURCES)

    get_filename_component(
        test_name
        "${test_src}"
        NAME_WE
    )

    add_executable(
        "${test_name}"
        "${test_src}"
    )

    target_link_libraries(
        "${test_name}"
        PRIVATE
            daedalux_lib
            GTest::gtest_main
    )

    gtest_discover_tests(
        ${test_name}
        WORKING_DIRECTORY
            "${TEST_FIXTURES_DIR}"
    )
            

endforeach()