# --------------------------------------------------------------------
# 2) CUDD integration as part of project via ExternalProject
# --------------------------------------------------------------------

include(ExternalProject)
set(CUDD_SRC     "${PROJECT_SOURCE_DIR}/src/libs/cudd")
set(CUDD_CFG     "${CUDD_SRC}/config.h")
# CUDD is configured and built in the build tree: the Makefile committed in src/libs/cudd is a stale generated file.
set(CUDD_BUILD   "${CMAKE_BINARY_DIR}/ext/cudd/build")

add_custom_target(bootstrap_cudd
    COMMENT "Bootstrapping CUDD (./configure -> config.h, .libs/...)"
    WORKING_DIRECTORY ${CUDD_SRC}
    COMMAND ./configure
      --enable-silent-rules
      --enable-obj
      --enable-dddmp
      --disable-shared
    # only re-run if configure.in/configure changes
    DEPENDS ${CUDD_SRC}/configure ${CUDD_SRC}/configure.ac
    # this makes sure config.h is created before anyone uses it:
    BYPRODUCTS ${CUDD_CFG}
)

# Allow CUDD headers (C++ wrapper and internal) to be found
include_directories(
    $<BUILD_INTERFACE:${CUDD_SRC}/cplusplus>
    $<BUILD_INTERFACE:${CUDD_SRC}>
    $<BUILD_INTERFACE:${CUDD_SRC}/cudd>
    $<BUILD_INTERFACE:${CUDD_SRC}/st>
    $<BUILD_INTERFACE:${CUDD_SRC}/mtr>
    $<BUILD_INTERFACE:${CUDD_SRC}/epd>
    $<BUILD_INTERFACE:${CUDD_SRC}/dddmp>
    $<BUILD_INTERFACE:${CUDD_BUILD}>
)

ExternalProject_Add(
  CUDD_project
  PREFIX          "${CMAKE_BINARY_DIR}/ext/cudd"
  SOURCE_DIR      "${CUDD_SRC}"
  BINARY_DIR      "${CUDD_BUILD}"
  # A git checkout gives these generated autotools files arbitrary timestamps, and make would then try to
  # regenerate them with the exact automake version that produced them. Touch them in dependency order first.
  CONFIGURE_COMMAND touch "${CUDD_SRC}/aclocal.m4" "${CUDD_SRC}/configure" "${CUDD_SRC}/config.h.in"
      "${CUDD_SRC}/Makefile.in"
    COMMAND "${CUDD_SRC}/configure"
      --enable-silent-rules
      --enable-obj
      --enable-dddmp
      --disable-shared
  BUILD_COMMAND make -j4
  INSTALL_COMMAND ""
  BUILD_BYPRODUCTS
    "${CUDD_BUILD}/cudd/.libs/libcudd.a"
)

add_library(CUDD::cudd STATIC IMPORTED GLOBAL)
set_target_properties(CUDD::cudd PROPERTIES
  IMPORTED_LOCATION
    "${CUDD_BUILD}/cudd/.libs/libcudd.a"
  INTERFACE_INCLUDE_DIRECTORIES
    "${CUDD_SRC}/cudd"
)
add_dependencies(CUDD::cudd CUDD_project)

# CUDD 3 with --enable-obj puts the C++ wrapper into libcudd.a; there is no separate libobj.a.
add_library(CUDD::obj STATIC IMPORTED GLOBAL)
set_target_properties(CUDD::obj PROPERTIES
  IMPORTED_LOCATION
    "${CUDD_BUILD}/cudd/.libs/libcudd.a"
  INTERFACE_INCLUDE_DIRECTORIES
    "${CUDD_SRC}/cplusplus"
)
add_dependencies(CUDD::obj CUDD_project)