# --------------------------------------------------------------------
# GoogleTest
# --------------------------------------------------------------------

include(FetchContent)

# GoogleTest 1.14.0, named by its commit: a tag can be moved.
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        f8d7d77c06936315286eb55f8de22cd23c188571
)

# GoogleTest serves the test build only: it is not installed or packaged with
# DaedaluX. No test uses GoogleMock.
set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF)

FetchContent_MakeAvailable(googletest)

# --------------------------------------------------------------------
# Test data
# --------------------------------------------------------------------

# All tests read tests/data from one copy in the build tree.
# Tests must not modify that copy: a case takes a private copy of a file it rewrites.
# The build redoes the shared copy when a file is edited, added or removed.

set(
    TEST_DATA_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/data"
)

set(
    TEST_DATA_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/data"
)

file(
    GLOB_RECURSE TEST_DATA
    CONFIGURE_DEPENDS
    "${TEST_DATA_SOURCE}/*"
)

# Changes when a file is added or removed, which no file timestamp shows.
file(
    GENERATE
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test_data.list"
    CONTENT "${TEST_DATA}"
)

# The stamp is inside the copy: removing the copy makes the build redo it.
add_custom_command(
    OUTPUT
        "${TEST_DATA_DIR}/.stamp"

    COMMAND
        "${CMAKE_COMMAND}" -E rm -rf "${TEST_DATA_DIR}"

    COMMAND
        "${CMAKE_COMMAND}" -E copy_directory "${TEST_DATA_SOURCE}" "${TEST_DATA_DIR}"

    COMMAND
        "${CMAKE_COMMAND}" -E touch "${TEST_DATA_DIR}/.stamp"

    DEPENDS
        ${TEST_DATA}
        "${CMAKE_CURRENT_BINARY_DIR}/test_data.list"

    COMMENT "Copying the test data"
    VERBATIM
)

add_custom_target(
    daedalux_test_data
    DEPENDS
        "${TEST_DATA_DIR}/.stamp"
)

# --------------------------------------------------------------------
# Test support
# --------------------------------------------------------------------

# The main shared by the test executables runs each case in a private, empty
# working directory below the runtime directory. A case writes its outputs
# there, and copies there the test data it modifies.

set(
    TEST_RUNTIME_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/runtime"
)

set(
    TEST_SUPPORT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/support/test_main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/support/TestEnvironment.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/support/TestPaths.cpp"
)

add_library(
    daedalux_test_support
    STATIC
        ${TEST_SUPPORT_SOURCES}
)

target_link_libraries(
    daedalux_test_support
    PUBLIC
        GTest::gtest
)

target_compile_definitions(
    daedalux_test_support
    PRIVATE
        DAEDALUX_TEST_DATA_DIR="${TEST_DATA_DIR}"
        DAEDALUX_TEST_RUNTIME_DIR="${TEST_RUNTIME_DIR}"
)

add_dependencies(
    daedalux_test_support
    daedalux_test_data
)

# --------------------------------------------------------------------
# Test targets
# --------------------------------------------------------------------

file(
    GLOB_RECURSE TEST_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

list(
    REMOVE_ITEM TEST_SOURCES
    ${TEST_SUPPORT_SOURCES}
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
            daedalux_test_support
    )

    gtest_discover_tests(
        ${test_name}
        PROPERTIES
            LABELS "${test_label}"
            TIMEOUT 120
    )
            

endforeach()

# CTest runs each case in its own process. This entry runs the cases of one
# executable together, where the workspace has to be renewed between them.

add_test(
    NAME TestWorkspaceTest.CasesInOneProcess
    COMMAND test_workspace
)

set_tests_properties(
    TestWorkspaceTest.CasesInOneProcess
    PROPERTIES
        LABELS integration
        TIMEOUT 120
)
