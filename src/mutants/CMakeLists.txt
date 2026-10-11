add_library(daedalux_mutants OBJECT)

target_sources(
    daedalux_mutants
    PRIVATE
        cli/mutant_subcommand.cpp 
        modelAnalyzer.cpp 
        mutantAnalyzer.cpp 
        spinRunner.cpp 
)

target_include_directories(
    daedalux_mutants
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(
    daedalux_mutants
    PUBLIC
        cxx_std_20
)

set_property(
    TARGET 
        daedalux_mutants
    PROPERTY 
        DAEDALUX_RUNTIME_DEPENDENCIES spin gcc
)

find_program(SPIN_EXECUTABLE NAMES spin)
find_program(GCC_EXECUTABLE NAMES gcc)

add_feature_info(
    MUTANTS_SPIN
    SPIN_EXECUTABLE
    "spin runtime runtime dependency for mutant analysis"
)

add_feature_info(
    MUTANTS_GCC
    GCC_EXECUTABLE
    "gcc runtime dependency for spin mutant analysis"
)