# cmake/Packaging.cmake

set(CPACK_PACKAGE_NAME "daedalux")

set(
    CPACK_PACKAGE_VERSION_MAJOR
    ${PROJECT_VERSION_MAJOR}
)

set(
    CPACK_PACKAGE_VERSION_MINOR
    ${PROJECT_VERSION_MINOR}
)

set(
    CPACK_PACKAGE_VERSION_PATCH
    ${PROJECT_VERSION_PATCH}
)

set(CPACK_GENERATOR "TGZ;ZIP")

set(
    CPACK_PACKAGE_CONTACT
    "sami.lazreg.suidi@gmail.com"
)

include(CPack)