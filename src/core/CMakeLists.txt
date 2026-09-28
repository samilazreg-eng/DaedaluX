add_library(daedalux_core OBJECT)

target_sources(
    daedalux_core
    PRIVATE
        ast/expr/condExpr.cpp
        ast/expr/expr.cpp
        ast/stmnt/stmnt.cpp
        ast/astNode.cpp

        automata/astToFsm.cpp
        automata/fsm.cpp 
        automata/fsmEdge.cpp
        automata/fsmNode.cpp
        automata/fsmToAst.cpp

        logic/ltl.cpp 

        semantic/variable/state/composite.cpp 
        semantic/variable/state/state.cpp 
        semantic/variable/state/stateDecorator.cpp 
        semantic/variable/state/thread.cpp
        semantic/variable/transition/compositeTransition.cpp 
        semantic/variable/transition/deleteTransVisitor.cpp 
        semantic/variable/transition/rendezVousTransition.cpp 
        semantic/variable/transition/threadTransition.cpp 
        semantic/variable/transition/transition.cpp 
        semantic/variable/variable.cpp
        semantic/variable/variableDecorator.cpp 
        semantic/payload.cpp

        symbol/symbol.cpp
        symbol/symTable.cpp 
)

target_include_directories(
    daedalux_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(
    daedalux_core
    PUBLIC
        cxx_std_20
)

