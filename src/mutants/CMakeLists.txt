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