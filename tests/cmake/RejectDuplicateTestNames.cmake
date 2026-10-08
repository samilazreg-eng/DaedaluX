# CTest loads this file after the cases of every test executable
# (TEST_INCLUDE_FILES in tests/CMakeLists.txt). gtest_discover_tests leaves
# the cases of an executable in the variable <target>_TESTS.

get_cmake_property(variables VARIABLES)

list(
    FILTER variables
    INCLUDE REGEX "_TESTS$"
)

set(names "")

foreach(variable IN LISTS variables)
    foreach(name IN LISTS ${variable})

        list(FIND names "${name}" index)

        if(NOT index EQUAL -1)
            message(
                FATAL_ERROR
                "Two test executables declare ${name}.\n"
                "Rename one of the two suites, then build again."
            )
        endif()

        list(APPEND names "${name}")

    endforeach()
endforeach()
