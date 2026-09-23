#include <daedalux/promela/parser/promela_loader.hpp>

#include <daedalux/core/automata/astToFsm.hpp>

#include "y.tab.hpp"
#include "lexer.h"

#include <assert.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include <unistd.h>

namespace fs = std::filesystem;

namespace {

// Scratch directories still alive, with the process that created them. The loader and the parser leave through
// exit(1) on errors, which skips ~promela_loader but runs static destructors: this one removes what is left.
struct ScratchDirs {
  std::map<fs::path, pid_t> live;
  ~ScratchDirs()
  {
    for (const auto & [dir, owner] : live)
      if (owner == getpid()) {
        std::error_code ignored;
        fs::remove_all(dir, ignored);
      }
  }
};

ScratchDirs & scratchDirs()
{
  static ScratchDirs dirs;
  return dirs;
}

// Each load works in its own directory, so concurrent loads never share working files.
fs::path makeScratchDir()
{
  auto pattern = (fs::temp_directory_path() / "daedalux-loader-XXXXXX").string();
  if (mkdtemp(pattern.data()) == nullptr) {
    std::cerr << "Could not create a temporary directory for the Promela loader." << std::endl;
    exit(1);
  }
  scratchDirs().live.emplace(pattern, getpid());
  return pattern;
}

// Single-quoted for sh, with embedded single quotes closed, escaped and reopened.
std::string shellQuoted(const fs::path & path)
{
  std::string quoted = "'";
  for (char c : path.string())
    quoted += c == '\'' ? std::string("'\\''") : std::string(1, c);
  return quoted + "'";
}

} // namespace

promela_loader::promela_loader(std::string file_name, const TVL * tvl)
    : globalSymTab(nullptr), program(nullptr), automata(nullptr), scratchDir(makeScratchDir())
{
  // A string without ".pml" is Promela source rather than a file name.
  fs::path sourcePath = file_name;
  if (file_name.find(".pml") == std::string::npos) {
    sourcePath = scratchDir / "__workingfile.tmp";
    std::ofstream(sourcePath) << file_name;
  }

  // Read the original file
  auto fileStream = std::make_shared<std::ifstream>(sourcePath);
  if (!fileStream->is_open()) {
    std::cerr << "The fPromela file does not exist or is not readable!" << std::endl;
    exit(1);
  }
  std::stringstream buffer;
  buffer << fileStream->rdbuf();

  // cpp reads the model from stdin in the working directory, as the old working copy did: #include "..." resolves
  // from the working directory, and line markers name "<stdin>", which has no digits for the lexer to take for a
  // line number.
  auto preprocessedFile = scratchDir / "__workingfile.tmp.cpp";
  auto preprocess = "cpp < " + shellQuoted(sourcePath) + " > " + shellQuoted(preprocessedFile);
  if (system(preprocess.c_str()) != 0) {
    std::cerr << "Could not run the c preprocessor (cpp)." << std::endl;
    exit(1);
  }

  // Open the temporary file
  yyin = fopen(preprocessedFile.c_str(), "r");
  if (yyin == nullptr) {
    std::cerr << "Could not open temporary working file (" << file_name << ")." << std::endl;
    exit(1);
  }
  init_lex();

  if (yyparse(&this->globalSymTab, &this->program) != 0) {
    std::cerr << "Syntax error; aborting." << std::endl;
    exit(1);
  }

  while (globalSymTab->prevSymTab())
    globalSymTab = globalSymTab->prevSymTab();

  // Create the converter
  std::unique_ptr<ASTtoFSM> converter = std::make_unique<ASTtoFSM>();
  // Create the automata from the AST
  automata = std::make_shared<fsm>(*converter->astToFsm(globalSymTab, program, tvl));

  std::ofstream graph;
  graph.open("fsm_graphvis");
  automata->printGraphVis(graph);
  graph.close();
}


extern std::string nameSpace;
extern symbol::Type declType;
extern tdefSymNode* typeDef;
extern mtypedefSymNode* mtypeDef;

extern symTable* currentSymTab;
extern symTable* savedSymTab;

extern std::list<varSymNode*> declSyms;
extern std::list<varSymNode*> typeLst;
extern std::list<std::string> params;
extern std::list<variantQuantifier*> variants;

extern std::map<std::string, stmntLabel*> labelsMap;

extern int mtypeId;
extern bool inInline;

promela_loader::~promela_loader(){
  nameSpace = "global";
  declType = symbol::T_NA;
  typeDef = nullptr;
  mtypeDef = nullptr;

  currentSymTab = nullptr;
  savedSymTab = nullptr;

  declSyms.clear();
  typeLst.clear();
  params.clear();
  variants.clear();

  labelsMap.clear();

  mtypeId = 1;
  inInline = false;

  if(yyin != nullptr) {
		fclose(yyin);
		yylex_destroy();
	}

  std::error_code ignored;
  fs::remove_all(scratchDir, ignored);
  scratchDirs().live.erase(scratchDir);
}