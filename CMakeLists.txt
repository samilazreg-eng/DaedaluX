cmake_minimum_required(VERSION 3.21)

project(
  daedalux
  VERSION 1.0.0
  LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS_COMMANDS ON)

# --------------------------------------------------------------------
# Project options
# --------------------------------------------------------------------


option(ENABLE_SANITIZERS "Enable Address/Undefined sanitizers" OFF)

# --------------------------------------------------------------------
# Language configuration
# --------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------

include(cmake/dependencies/CUDD.cmake)

# --------------------------------------------------------------------
# Product
# --------------------------------------------------------------------

add_subdirectory(src)

# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------

include(CTest)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

# --------------------------------------------------------------------
# Installation & packaging
# --------------------------------------------------------------------

include(cmake/Install.cmake)
include(cmake/Packaging.cmake)