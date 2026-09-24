#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

#include <daedalux/core/automata.hpp>

#include <daedalux/algorithm/utils/bisimulationChecker.hpp>


// Define a fixture for the tests
class BisimilarityTest : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override
  {
    // Common teardown code that will be called after each test
  }
  std::string test1 = "/test_files/mutants/array.pml";
  std::string test1_mutant = "/test_files/mutants/array_mutant.pml";
  std::string current_path = std::filesystem::current_path();
};


// TEST_F(BisimilarityTest, AnFSMShouldBeBisimilarToItself)
// {
//   const TVL * tvl = nullptr;
//   auto file_path = current_path + test1;
//   auto original_loader = std::make_unique<promela_loader>(file_path, tvl);
//   auto originalFSM = original_loader->getAutomata();

//   ASSERT_TRUE(BisimulationChecker::isBisimilar(originalFSM, originalFSM));
// }

// TEST_F(BisimilarityTest, TwoFSMAreNotBisimilar)
// {
//   const TVL * tvl = nullptr;
//   auto file_path = current_path + test1;
//   auto original_loader = new promela_loader(file_path, tvl);
//   auto originalFSM = original_loader->getAutomata();
//   delete original_loader;
//   auto file_path_mutant = current_path + test1_mutant;
//   auto mutant_loader = std::make_unique<promela_loader>(file_path_mutant, tvl);
//   auto mutantFSM = mutant_loader->getAutomata();
//   ASSERT_FALSE(BisimulationChecker::isBisimilar(originalFSM, mutantFSM));
// }
