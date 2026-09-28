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