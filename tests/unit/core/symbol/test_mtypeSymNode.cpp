#include <gtest/gtest.h>
#include <daedalux/promela/symbol/vardef/mtypeSymNode.hpp>

class MtypeSymNodeTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }

  std::string foo = "/test_files/basic/hello_world.pml";
  std::string minepump = "/models/minepump/minepump.pml";
  std::string minepump_mutant = "/models/minepump/mutants/mutant_1.pml";
  std::string current_path = std::filesystem::current_path();
};

TEST_F(MtypeSymNodeTest, CompareMtypesValuesDiffFSM)
{
  const TVL * tvl = nullptr;
  auto file_path = current_path + minepump;
  auto original_loader = std::make_unique<promela_loader>(file_path, tvl);
  auto originalFSM = original_loader->getAutomata();
  // Load the mutant
  auto file_path_mutant = current_path + minepump_mutant;
  auto mutant_loader = std::make_unique<promela_loader>(file_path_mutant, tvl);
  auto mutantFSM = mutant_loader->getAutomata();
  // Create the initial state for both automata
  generateNegativeTraces(originalFSM, mutantFSM, 10);
}