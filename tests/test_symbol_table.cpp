#include <gtest/gtest.h>
#include <memory>
#include <filesystem>
#include "../src/promela/parser/promela_loader.hpp"

using namespace std;
namespace fs = std::filesystem;

// Define a fixture for the tests
class SymbolTableTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup code that will be called before each test

    }

    void TearDown() override {
        // Common teardown code that will be called after each test
    }
};

// Test case for loading an invalid Promela file
TEST_F(SymbolTableTest, LoadValidPromelaFile) {

    std::string current_directory = fs::current_path();
    std::string file_name = "/test_files/basic/array.pml";
    std::string file_path = current_directory + file_name;
    const TVL* tvl = nullptr;
    auto loader = std::make_unique<promela_loader>(file_path, tvl);

    auto symbolTable = loader->getSymTable()->getSubSymTab("global");

    symbolTable->print();
    auto test = symbolTable->lookup("test");
    test->setName("software");

    auto program = loader->getProgram();
    std::cout << stmnt::string(program) << std::endl;

    auto array2 = symbolTable->lookup("array");
    array2->setName("array2");

    symbolTable->print();
    std::cout << stmnt::string(program) << std::endl;
    


}