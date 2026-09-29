# --------------------------------------------------------------------
# 1) Parser generation (Flex & Bison)
# --------------------------------------------------------------------
set(PARSER_Y   
    "${CMAKE_CURRENT_SOURCE_DIR}/promela.y"
)

set(LEXER_L    
    "${CMAKE_CURRENT_SOURCE_DIR}/promela.l"
)

set(PARSER_CPP 
    "${CMAKE_CURRENT_SOURCE_DIR}/y.tab.cpp"
)

set(LEXER_CPP  
    "${CMAKE_CURRENT_SOURCE_DIR}/lex.yy.cpp"
)

add_custom_command(
    OUTPUT 
        ${PARSER_CPP}
    
    COMMAND 
        yacc -y -d -o ${PARSER_CPP} ${PARSER_Y}
    
    DEPENDS 
        ${PARSER_Y}
    
    COMMENT 
        "[Parser] Generating y.tab.cpp"
)

add_custom_command(
    OUTPUT 
        ${LEXER_CPP}
    
    COMMAND 
        flex -o ${LEXER_CPP} ${LEXER_L}
    
    DEPENDS 
        ${LEXER_L}
    
    COMMENT 
        "[Lexer] Generating lex.yy.cpp"
)

# Make parser generation a standalone target
add_custom_target(
    promela_parser
    DEPENDS 
        ${PARSER_CPP} 
        ${LEXER_CPP}
    
    COMMENT "Generate Promela parser sources"
)