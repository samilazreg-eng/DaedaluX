cmake_minimum_required(VERSION 3.21)
project(daedalux_tests LANGUAGES CXX)

enable_testing()

# Pull in GoogleTest from the main project:
find_package(GTest REQUIRED)
find_package(daedalux REQUIRED)   # so we get daedalux::daedalux_lib

add_subdirectory(unit)
add_subdirectory(integration)

