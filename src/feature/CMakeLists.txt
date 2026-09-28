add_library(daedalux_feature OBJECT)

target_sources(
    daedalux_feature
    PRIVATE
        semantic/variable/state/featured.cpp 
        semantic/variable/transition/featuredTransition.cpp
        ADDutils.cpp 
        expToADD.cpp 
        tvl.cpp
)


target_include_directories(
    daedalux_feature
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(
    daedalux_feature
    PUBLIC
        cxx_std_20
)
