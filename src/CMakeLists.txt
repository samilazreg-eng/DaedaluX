# Components of DaedaluX

add_subdirectory(algorithm)
add_subdirectory(core)
add_subdirectory(feature)
add_subdirectory(formulas)
add_subdirectory(mutants)
add_subdirectory(promela)
add_subdirectory(visualizer)

add_library(
    daedalux_lib 
    STATIC
        $<TARGET_OBJECTS:daedalux_algorithm>
        $<TARGET_OBJECTS:daedalux_core>
        $<TARGET_OBJECTS:daedalux_feature>
        $<TARGET_OBJECTS:daedalux_formulas>
        $<TARGET_OBJECTS:daedalux_mutants>
        $<TARGET_OBJECTS:daedalux_promela>
        $<TARGET_OBJECTS:daedalux_visualizer>
)

add_dependencies(daedalux_core CUDD_project)
add_dependencies(daedalux_algorithm CUDD_project)
add_dependencies(daedalux_feature CUDD_project)
add_dependencies(daedalux_formulas CUDD_project)
add_dependencies(daedalux_promela CUDD_project)
add_dependencies(daedalux_mutants CUDD_project)
add_dependencies(daedalux_visualizer CUDD_project)

target_link_libraries(
    daedalux_lib
    PUBLIC
        CUDD::obj
        CUDD::cudd
)

target_include_directories(
    daedalux_lib 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
)

target_compile_features(
    daedalux_lib
    PUBLIC
        cxx_std_20
)

# CLI

add_executable(
    daedalux_cli 
    main_cli.cpp
)

target_link_libraries(
    daedalux_cli
    PRIVATE
        daedalux_lib
        CUDD::obj 
        CUDD::cudd
)
