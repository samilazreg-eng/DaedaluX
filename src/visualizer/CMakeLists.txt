add_library(daedalux_visualizer OBJECT)

target_sources(
    daedalux_visualizer
    PRIVATE 
        stateToGraphViz.cpp 
        trace.cpp 
        traceReport.cpp 
)

target_include_directories(
    daedalux_visualizer 
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(
    daedalux_visualizer
    PUBLIC
        cxx_std_20
)