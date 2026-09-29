install(
    TARGETS
        daedalux_lib
        daedalux_cli
    EXPORT
        daedaluxTargets
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(
    DIRECTORY
        "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION
        include
)

install(
    EXPORT
        daedaluxTargets
    FILE
        daedaluxTargets.cmake
    NAMESPACE
        daedalux::
    DESTINATION
        lib/cmake/daedalux
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/daedaluxConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/daedaluxConfig.cmake"
    INSTALL_DESTINATION
        lib/cmake/daedalux
)

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/daedaluxConfigVersion.cmake"
    VERSION
        ${PROJECT_VERSION}
    COMPATIBILITY
        AnyNewerVersion
)

install(
    FILES
        "${PROJECT_BINARY_DIR}/daedaluxConfig.cmake"
        "${PROJECT_BINARY_DIR}/daedaluxConfigVersion.cmake"
    DESTINATION
        lib/cmake/daedalux
)