#include <daedalux/algorithm.hpp>
#include <daedalux/core/ast.hpp>
#include <daedalux/core/symbol.hpp>
#include <daedalux/core/automata.hpp>

// instead of including y.tab.hpp and lexer.h directly,
// just pull in the Promela parser via the loader
#include <daedalux/promela/parser/promela_loader.hpp>


#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

#include "../../src/promela/parser/y.tab.hpp"
#include "../../src/promela/parser/lexer.h"

namespace fs = std::filesystem;

// Define a fixture for the tests
class DISABLED_SpecificationWriterTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    // Common setup code that will be called before each test
  }

  void TearDown() override
  {
    // Common teardown code that will be called after each test
  }
};

extern void init_lex();

bool compare_original_and_written_programs(std::string file_name)
{
  symTable * globalSymTab;
  stmnt * program;
  // The variable promelaFile should have the fileExtension .pml
  if (file_name.find(".pml") == std::string::npos) {
    std::cerr << "The model file must have the extension .pml." << std::endl;
    exit(1);
  }
  // Copy the model file to a temporary file
  fs::path sourcePath = file_name;
  std::string current_directory = fs::current_path();
  auto destinationFile = current_directory + "/__workingfile.tmp";
  fs::path destinationPath = destinationFile;
  try {
    fs::copy(sourcePath, destinationPath, fs::copy_options::overwrite_existing);
  }
  catch (const fs::filesystem_error & e) {
    std::cerr << "Error: " << e.what() << std::endl;
    std::cerr << "The fPromela file does not exist or is not readable!" << std::endl;
    exit(1);
  }
  if (system("cpp __workingfile.tmp __workingfile.tmp.cpp") != 0) {
    std::cerr << "Could not run the c preprocessor (cpp)." << std::endl;
    exit(1);
  }
  // Read the original file
  auto fileStream = std::make_shared<std::ifstream>(sourcePath);
  if (!fileStream->is_open()) {
    std::cerr << "The fPromela file does not exist or is not readable!" << std::endl;
    exit(1);
  }
  std::stringstream buffer;
  buffer << fileStream->rdbuf();

  printf("File read\n");

  // Open the temporary file
  yyin = fopen("__workingfile.tmp.cpp", "r");
  if (yyin == nullptr) {
    std::cerr << "Could not open temporary working file (" << file_name << ")." << std::endl;
    exit(1);
  }

  printf("Parsing the file\n");
  init_lex();
  if (yyparse(&globalSymTab, &program) != 0) {
    std::cerr << "Syntax error; aborting." << std::endl;
    exit(1);
  }

  if (yyin != nullptr) {
    fclose(yyin);
    yylex_destroy();
  }
  printf("Program parsed\n");
  printf("Comparing the original and written programs\n");
  return buffer.str() == stmnt::string(program);
}

TEST_F(DISABLED_SpecificationWriterTest, WriteSimpleSpecification)
{
  std::string current_directory = std::filesystem::current_path();
  std::string file_name = "/test_files/basic/flows.pml";
  std::string file_path = current_directory + file_name;
  ASSERT_TRUE(compare_original_and_written_programs(file_path));
}

TEST_F(DISABLED_SpecificationWriterTest, WriteSimpleSpecification_Array)
{
  std::string current_directory = std::filesystem::current_path();
  std::string file_name = "/test_files/basic/array.pml";
  std::string file_path = current_directory + file_name;
  ASSERT_TRUE(compare_original_and_written_programs(file_path));
}

TEST_F(DISABLED_SpecificationWriterTest, WriteMinepumpSpecification)
{
  std::string current_directory = std::filesystem::current_path();
  std::string file_name = "/models/minepump/original.pml";
  std::string file_path = current_directory + file_name;
  ASSERT_TRUE(compare_original_and_written_programs(file_path));
}