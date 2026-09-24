#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <memory>

#include <daedalux/promela/symbol.hpp>
#include <daedalux/promela/ast.hpp>

#include <daedalux/core/automata.hpp>
#include <daedalux/core/symbol.hpp>
#include <daedalux/core/ast.hpp>

#include <daedalux/feature/tvl.hpp>

extern void init_lex();

class promela_loader {
    public:
        promela_loader(std::string file_name, const TVL *tvl = nullptr);

        ~promela_loader();

        std::shared_ptr<fsm> getAutomata(void) const { return automata; }

        symTable * getSymTable(void) const { return globalSymTab; }

        stmnt * getProgram(void) const { return program; }

    private:

        symTable* globalSymTab;
        stmnt* program;
        std::shared_ptr<fsm> automata;
        std::filesystem::path scratchDir;
};