include(FeatureSummary)

add_library(daedalux_ltl INTERFACE)

target_sources(
    daedalux_ltl
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/ltl.cpp
)

set_property(
    TARGET 
        daedalux_ltl
    PROPERTY 
        DAEDALUX_RUNTIME_DEPENDENCIES ltl2ba
)

find_program(LTL2BA_EXECUTABLE NAMES ltl2ba)

add_feature_info(
    LOGIC_LTL2BA
    LTL2BA_EXECUTABLE
    "ltl2ba runtime dependency for LTL formula processing"
)