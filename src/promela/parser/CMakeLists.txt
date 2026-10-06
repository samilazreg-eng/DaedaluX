# --------------------------------------------------------------------
# 1) Parser generation (Flex & Bison)
# --------------------------------------------------------------------

option(
    DAEDALUX_REGENERATE_PROMELA_PARSER
    "Enable the promela parser regeneration target"
    OFF
)

set(PARSER_Y "${CMAKE_CURRENT_SOURCE_DIR}/promela.y")
set(LEXER_L "${CMAKE_CURRENT_SOURCE_DIR}/promela.l")
set(PARSER_CPP "${CMAKE_CURRENT_SOURCE_DIR}/y.tab.cpp")
set(LEXER_CPP "${CMAKE_CURRENT_SOURCE_DIR}/lex.yy.cpp")


if(DAEDALUX_REGENERATE_PROMELA_PARSER)

    find_package(BISON 3.8.2 REQUIRED)
    find_package(FLEX 2.6.4 REQUIRED)

    add_custom_target(
        regenerate_promela_parser

        COMMAND
            "${BISON_EXECUTABLE}"
            -y -d
            -o "${PARSER_CPP}"
            "${PARSER_Y}"

        COMMAND
            "${FLEX_EXECUTABLE}"
            -o "${LEXER_CPP}"
            "${LEXER_L}"

        WORKING_DIRECTORY
            "${CMAKE_CURRENT_SOURCE_DIR}"

        COMMENT
            "Regenerating committed Promela parser sources"

        VERBATIM
    )

endif()

add_library(promela_parser INTERFACE)

target_sources(
    promela_parser
    INTERFACE
        ${PARSER_CPP}
        ${LEXER_CPP}
        ${CMAKE_CURRENT_SOURCE_DIR}/promela_loader.cpp
)
