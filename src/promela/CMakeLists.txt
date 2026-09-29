add_library(daedalux_promela OBJECT)

add_subdirectory(parser)

target_sources(
    daedalux_promela
    PRIVATE
        ast/expr/argExpr.cpp 
        ast/expr/binaryExpr.cpp 
        ast/expr/constExpr.cpp 
        ast/expr/runExpr.cpp 
        ast/expr/skipExpr.cpp 
        ast/expr/timeoutExpr.cpp 
        ast/expr/unaryExpr.cpp 
        ast/expr/varExpr.cpp
        ast/stmnt/chanStmnt.cpp 
        ast/stmnt/clockStmnt.cpp 
        ast/stmnt/decl.cpp
        ast/stmnt/flowStmnt.cpp
        ast/stmnt/logicDecl.cpp 
        ast/stmnt/stdlibStmnt.cpp

        semantic/variable/state/initState.cpp 
        semantic/variable/state/never.cpp 
        semantic/variable/state/process.cpp 
        semantic/variable/state/program.cpp 
        semantic/variable/transition/progTransition.cpp 
        semantic/variable/boolVariable.cpp 
        semantic/variable/channel.cpp 
        semantic/variable/mtypeVariable.cpp 
        semantic/variable/primitiveVariable.cpp 
        semantic/variable/utypeVariable.cpp

        symbol/logic/temporalSymNode.cpp 
        symbol/typedef/inlineSymNode.cpp 
        symbol/typedef/mtypedefSymNode.cpp 
        symbol/typedef/ptypeSymNode.cpp 
        symbol/typedef/tdefSymNode.cpp 
        symbol/vardef/bitSymNode.cpp 
        symbol/vardef/boolSymNode.cpp 
        symbol/vardef/byteSymNode.cpp 
        symbol/vardef/chanSymNode.cpp
        symbol/vardef/cidSymNode.cpp 
        symbol/vardef/intSymNode.cpp 
        symbol/vardef/mtypeSymNode.cpp 
        symbol/vardef/naSymNode.cpp 
        symbol/vardef/pidSymNode.cpp 
        symbol/vardef/shortSymNode.cpp 
        symbol/vardef/sysSymNode.cpp 
        symbol/vardef/unsgnSymNode.cpp 
        symbol/vardef/utypeSymNode.cpp 
        symbol/vardef/variantSymNode.cpp 
        symbol/vardef/varSymNode.cpp
)

target_include_directories(
    daedalux_promela 
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
    daedalux_promela
    PRIVATE
)

target_compile_features(
    daedalux_promela
    PUBLIC 
        cxx_std_20
)