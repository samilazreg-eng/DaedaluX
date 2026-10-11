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

set_property(
    TARGET 
        daedalux_feature
    PROPERTY 
        DAEDALUX_RUNTIME_DEPENDENCIES java TVLParser.jar
)

find_program(JAVA_EXECUTABLE NAMES java)

add_feature_info(
    FEATURE_JAVA
    JAVA_EXECUTABLE
    "java runtime dependency for TVL parser processing"
)


find_file(TVL_PARSER_JAR
    NAMES TVLParser.jar
    PATHS_SUFFIXES
        share
)

add_feature_info(
    FEATURE_TVL_PARSER
    TVL_PARSER_JAR
    "TVL parser runtime dependency for TVL processing"
)