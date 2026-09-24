cmake_minimum_required(VERSION 3.19)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(daedalux VERSION 1.0.0 LANGUAGES CXX)

# Grouping in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Options
option(BUILD_TESTING "Build unit tests" ON)
option(BUILD_EXAMPLES "Build example binaries" OFF)
option(ENABLE_SANITIZERS "Enable Address/Undefined sanitizers" OFF)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------------------------
# 1) Parser generation (Flex & Bison)
# --------------------------------------------------------------------
set(PROMELA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/promela")
set(PARSER_Y   "${PROMELA_DIR}/parser/promela.y")
set(LEXER_L    "${PROMELA_DIR}/parser/promela.l")
set(PARSER_CPP "${PROMELA_DIR}/parser/y.tab.cpp")
set(LEXER_CPP  "${PROMELA_DIR}/parser/lex.yy.cpp")

add_custom_command(
    OUTPUT ${PARSER_CPP}
    COMMAND yacc -y -d -o ${PARSER_CPP} ${PARSER_Y}
    DEPENDS ${PARSER_Y}
    COMMENT "[Parser] Generating y.tab.cpp"
)
add_custom_command(
    OUTPUT ${LEXER_CPP}
    COMMAND flex -o ${LEXER_CPP} ${LEXER_L}
    DEPENDS ${LEXER_L}
    COMMENT "[Lexer] Generating lex.yy.cpp"
)

# Make parser generation a standalone target
add_custom_target(promela_parser
    DEPENDS ${PARSER_CPP} ${LEXER_CPP}
    COMMENT "Generate Promela parser sources"
)

# --------------------------------------------------------------------
# 2) CUDD integration as part of project via ExternalProject
# --------------------------------------------------------------------

include(ExternalProject)
set(CUDD_SRC     "${CMAKE_SOURCE_DIR}/src/libs/cudd")
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

# --------------------------------------------------------------------
# 3) Module-based object libraries
# --------------------------------------------------------------------

# Core module
file(GLOB_RECURSE CORE_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp"
)
add_library(daedalux_core OBJECT ${CORE_SOURCES})
target_include_directories(daedalux_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

# Algorithm module
file(GLOB_RECURSE ALG_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/algorithm/*.cpp"
)
add_library(daedalux_algorithm OBJECT ${ALG_SOURCES})
target_include_directories(daedalux_algorithm PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

# Feature module
file(GLOB_RECURSE FEAT_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/feature/*.cpp"
)
add_library(daedalux_feature OBJECT ${FEAT_SOURCES})
target_include_directories(daedalux_feature PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

# Formulas module
file(GLOB_RECURSE FORM_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/formulas/*.cpp"
)
add_library(daedalux_formulas OBJECT ${FORM_SOURCES})
target_include_directories(daedalux_formulas PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

# Promela module
file(GLOB_RECURSE PROM_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/promela/*.cpp"
)
add_library(daedalux_promela OBJECT ${PROM_SOURCES} ${PARSER_CPP} ${LEXER_CPP})
# Ensure the Promela parser is generated before this module
add_dependencies(daedalux_promela promela_parser)
target_include_directories(daedalux_promela PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

# Mutants module
file(GLOB_RECURSE MUT_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mutants/*.cpp"
)
add_library(daedalux_mutants OBJECT ${MUT_SOURCES})
target_include_directories(daedalux_mutants PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

# Visualizer module
file(GLOB_RECURSE VIS_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/visualizer/*.cpp"
)
add_library(daedalux_visualizer OBJECT ${VIS_SOURCES})
target_include_directories(daedalux_visualizer PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

# --------------------------------------------------------------------
# 4) Core static library
# --------------------------------------------------------------------

add_library(daedalux_lib STATIC
  $<TARGET_OBJECTS:daedalux_core>
  $<TARGET_OBJECTS:daedalux_algorithm>
  $<TARGET_OBJECTS:daedalux_feature>
  $<TARGET_OBJECTS:daedalux_formulas>
  $<TARGET_OBJECTS:daedalux_promela>
  $<TARGET_OBJECTS:daedalux_mutants>
  $<TARGET_OBJECTS:daedalux_visualizer>
)
add_dependencies(daedalux_lib CUDD_project)
# The modules include CUDD headers that need the config.h generated by CUDD's configure step.
add_dependencies(daedalux_core CUDD_project)
add_dependencies(daedalux_algorithm CUDD_project)
add_dependencies(daedalux_feature CUDD_project)
add_dependencies(daedalux_formulas CUDD_project)
add_dependencies(daedalux_promela CUDD_project)
add_dependencies(daedalux_mutants CUDD_project)
add_dependencies(daedalux_visualizer CUDD_project)
target_link_libraries(daedalux_lib PUBLIC
    CUDD::obj
    CUDD::cudd
)

target_include_directories(daedalux_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
)

if(ENABLE_SANITIZERS)
    target_compile_options(daedalux_lib PRIVATE
        -fsanitize=address,undefined -fno-omit-frame-pointer
    )
    target_link_libraries(daedalux_lib PRIVATE
        -fsanitize=address,undefined
    )
endif()

# --------------------------------------------------------------------
# 5) Executables
# --------------------------------------------------------------------

add_executable(daedalux_cli src/main_cli.cpp)
target_link_libraries(daedalux_cli PRIVATE
    daedalux_lib
    CUDD::obj
    CUDD::cudd
)
# --------------------------------------------------------------------
# 6) Testing
# --------------------------------------------------------------------
if(BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
  )
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
  foreach(test_src IN LISTS TEST_SOURCES)
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE daedalux_lib GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(${test_name})
  endforeach()
endif()

# --------------------------------------------------------------------
# 7) Install & Packaging
# --------------------------------------------------------------------
install(TARGETS daedalux_lib daedalux_cli
    EXPORT daedaluxTargets
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)

include(CPack)
set(CPACK_PACKAGE_NAME    "daedalux")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_GENERATOR        "TGZ;ZIP")
set(CPACK_PACKAGE_CONTACT  "sami.lazreg.suidi@gmail.com")

install(EXPORT daedaluxTargets
    FILE daedaluxTargets.cmake
    NAMESPACE daedalux::
    DESTINATION lib/cmake/daedalux
)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/daedaluxConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/daedaluxConfig.cmake"
    INSTALL_DESTINATION lib/cmake/daedalux
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/daedaluxConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
install(
    FILES
      "${CMAKE_CURRENT_BINARY_DIR}/daedaluxConfig.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/daedaluxConfigVersion.cmake"
    DESTINATION lib/cmake/daedalux
)

