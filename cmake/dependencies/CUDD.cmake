# CUDD is provided by the environment as an installed CMake package (README, "CUDD").
# DaedaluX locates it and consumes it: it does not fetch, configure or build it.
set(_cudd_requested_version 4.0.0)
find_package(cudd ${_cudd_requested_version} CONFIG QUIET)

if(NOT cudd_FOUND)
    # CMake records each package file it located, with the version that package declares.
    # No record: no package was located. Otherwise every located package refused the request.
    if(NOT cudd_CONSIDERED_CONFIGS)
        string(CONCAT _cudd_problem
            "CUDD could not be located. DaedaluX requires a compatible CUDD package "
            "(requested version: ${_cudd_requested_version}), and CMake found no "
            "cuddConfig.cmake or cudd-config.cmake in its search paths.\n"
        )
    else()
        set(_cudd_problem "")
        foreach(_cudd_config _cudd_config_version
                IN ZIP_LISTS cudd_CONSIDERED_CONFIGS cudd_CONSIDERED_VERSIONS)
            if(_cudd_config_version VERSION_LESS _cudd_requested_version)
                string(CONCAT _cudd_mismatch
                    "it is older than the requested version ${_cudd_requested_version} "
                    "and does not satisfy the required compatibility"
                )
            elseif(_cudd_config_version VERSION_GREATER _cudd_requested_version)
                string(CONCAT _cudd_mismatch
                    "this newer version does not satisfy the project's package compatibility "
                    "requirement (requested version: ${_cudd_requested_version})"
                )
            else()
                set(_cudd_mismatch
                    "its package does not accept the requested version ${_cudd_requested_version}"
                )
            endif()
            string(APPEND _cudd_problem
                "CUDD ${_cudd_config_version} was found at ${_cudd_config}, "
                "but ${_cudd_mismatch}.\n"
            )
        endforeach()
    endif()

    message(FATAL_ERROR
        "${_cudd_problem}"
        "To install a compatible CUDD, run the provided script with a prefix of your choice, "
        "then configure with that prefix, for example:\n"
        "  scripts/install-cudd.sh <prefix>\n"
        "  cmake -S . -B build -DCMAKE_PREFIX_PATH=<prefix>\n"
        "To use a compatible CUDD that is already installed, add its installation prefix to "
        "CMAKE_PREFIX_PATH, or set cudd_DIR to the directory that contains its "
        "cuddConfig.cmake.\n"
        "Prerequisites and details: README.md, section \"CUDD\"."
    )
endif()
