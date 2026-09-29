add_library(daedalux_algorithm OBJECT)

target_sources(
    daedalux_algorithm
    PRIVATE 
        cli/modelchecking_subcommand.cpp 
        utils/bisimulationChecker.cpp 
        utils/stateComparer.cpp
        algorithm.cpp 
        elementStack.cpp 
        explore.cpp 
        fsmExplorer.cpp 
        ltlModelChecker.cpp 
        reachabilityRelation.cpp 
        traceGenerator.cpp
)

target_include_directories(
    daedalux_algorithm
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(
    daedalux_algorithm
    PUBLIC
        cxx_std_20
)