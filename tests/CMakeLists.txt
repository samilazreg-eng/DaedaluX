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
# Suites that rewrite inputs use TestWorkspace to copy this input tree per case.

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
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

include(GoogleTest)

foreach(test_src IN LISTS TEST_SOURCES)

    get_filename_component(
        test_name
        "${test_src}"
        NAME_WE
    )

    # The directory below tests/ is the test's label.
    file(
        RELATIVE_PATH test_rel
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${test_src}"
    )

    string(
        REGEX MATCH "^[^/]+"
        test_label
        "${test_rel}"
    )

    if(NOT test_label MATCHES "^(unit|integration)$")
        message(
            FATAL_ERROR
            "${test_rel}: test sources belong in tests/unit or tests/integration"
        )
    endif()

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

    target_compile_definitions(
        "${test_name}"
        PRIVATE DAEDALUX_TEST_FIXTURES_DIR="${TEST_FIXTURES_DIR}"
    )

    gtest_discover_tests(
        ${test_name}
        WORKING_DIRECTORY
            "${TEST_FIXTURES_DIR}"
        PROPERTIES
            LABELS "${test_label}"
            TIMEOUT 120
    )
            

endforeach()
