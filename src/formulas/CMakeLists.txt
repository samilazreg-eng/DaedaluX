add_library(daedalux_formulas OBJECT)

target_sources(
    daedalux_formulas
    PRIVATE
        formulaCreator.cpp
        formulaUtility.cpp 
)

target_include_directories(
    daedalux_formulas
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(
    daedalux_formulas
    PUBLIC
        cxx_std_20
)
