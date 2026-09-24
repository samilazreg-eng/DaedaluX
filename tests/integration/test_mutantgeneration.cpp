#include <daedalux/mutants.hpp>

#include "../TestFilesUtils.hpp"

#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

// Define a fixture for the tests
class MutantGenerationTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    // Common setup code that will be called before each test
    std::string current_path = std::filesystem::current_path();
    testFiles = std::make_unique<TestFilesUtils>(current_path);
  }

  void TearDown() override
  {
    // Common teardown code that will be called after each test
  }

  std::unique_ptr<TestFilesUtils> testFiles;

  bool testMutantGeneration(std::string original_file_path, int number_of_mutants, bool shouldBeLess = false)
  {
    LTLClaimsProcessor::removeClaimFromFile(original_file_path);
    MutantAnalyzer mutantAnalyzer(original_file_path);
    mutantAnalyzer.createMutants(number_of_mutants);
    auto mutants = mutantAnalyzer.getMutantFilePaths();
    bool result = mutants.size() == number_of_mutants;
    bool isLess = mutants.size() < number_of_mutants;
    // Remove the mutants
    for (auto mutant : mutants) {
      std::filesystem::remove(mutant);
    }
    if (shouldBeLess)
    {
      return isLess;
    }
    return result;
  }
};

TEST_F(MutantGenerationTest, GenerateMutants)
{
  auto original_file_path = testFiles->array_model_never();
  int number_of_mutants = 5;
  ASSERT_TRUE(testMutantGeneration(original_file_path, number_of_mutants));
}

TEST_F(MutantGenerationTest, GenerateMutantsFlows)
{
  auto original_file_path = testFiles->flows_model_original();
  int number_of_mutants = 5;
  ASSERT_TRUE(testMutantGeneration(original_file_path, number_of_mutants));
}

TEST_F(MutantGenerationTest, GenerateManyMutantsFlows)
{
  auto original_file_path = testFiles->flows_model_original();
  int number_of_mutants = 100;
  bool shouldBeLess = true;
  ASSERT_TRUE(testMutantGeneration(original_file_path, number_of_mutants, shouldBeLess));
}

TEST_F(MutantGenerationTest, GenerateManyMutantsArray)
{
  auto original_file_path = testFiles->array_model_original();
  int number_of_mutants = 100;
  bool shouldBeLess = true;
  ASSERT_TRUE(testMutantGeneration(original_file_path, number_of_mutants, shouldBeLess));
}

TEST_F(MutantGenerationTest, GenerateManyMutants3Processes)
{
  auto original_file_path = testFiles->process_model_original();
  int number_of_mutants = 100;
  bool shouldBeLess = true;
  ASSERT_TRUE(testMutantGeneration(original_file_path, number_of_mutants, shouldBeLess));
}

TEST_F(MutantGenerationTest, GenerateMutantsMinepump)
{
  auto original_file_path = testFiles->minepump_model_original();
  int number_of_mutants = 5;
  ASSERT_TRUE(testMutantGeneration(original_file_path, number_of_mutants));
}