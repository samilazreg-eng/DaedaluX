#include <daedalux/core/semantic.hpp>
#include <daedalux/formulas.hpp>
#include <daedalux/mutants.hpp>
#include "../TestFilesUtils.hpp"

#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

// Define a fixture for the tests
class MutantHandlerTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    // Common setup code that will be called before each test
    std::string current_path = std::filesystem::current_path();
    testFilesUtils = std::make_unique<TestFilesUtils>(current_path);
  }

  void TearDown() override
  {
    // Common teardown code that will be called after each test
  }

  std::unique_ptr<TestFilesUtils> testFilesUtils;

  bool testEnhanceSpecification(std::string original_file_path, std::vector<std::string> mutant_files,
                                unsigned int number_of_mutants)
  {
    LTLClaimsProcessor::removeClaimFromFile(original_file_path);
    for (auto mutant_file_path : mutant_files) {
      LTLClaimsProcessor::removeClaimFromFile(mutant_file_path);
    }
    MutantAnalyzer mutantAnalyzer(original_file_path, mutant_files);
    auto formula = mutantAnalyzer.enhanceSpecification(number_of_mutants);
    auto [killed_mutants, alive_mutants] = mutantAnalyzer.killMutantsSpin(formula);
    return alive_mutants.empty();
  }
};

// TEST_F(MutantHandlerTest, mutant_generation_array)
// {
//   auto original_file_path = testFilesUtils->array_model_never();
//   MutantAnalyzer mutantAnalyzer(original_file_path);
//   auto number_of_mutants = 5;
//   mutantAnalyzer.createMutants(number_of_mutants);
//   auto mutants = mutantAnalyzer.getMutantFilePaths();
//   ASSERT_EQ(mutants.size(), number_of_mutants);

//   // Assert that all mutants are created
//   for (auto mutant : mutants) {
//     std::ifstream file(mutant);
//     ASSERT_TRUE(file.good());
//     // Ensure that the program is different from the original
//     std::ifstream original_file(original_file_path);
//     std::string original_program((std::istreambuf_iterator<char>(original_file)), std::istreambuf_iterator<char>());
//     std::ifstream mutant_file(mutant);
//     std::string mutant_program((std::istreambuf_iterator<char>(mutant_file)), std::istreambuf_iterator<char>());
//     ASSERT_NE(original_program, mutant_program);
//     // Remove the file
//     std::filesystem::remove(mutant);
//   }
// }

// TEST_F(MutantHandlerTest, mutant_generation_flows)
// {
//   auto original_file_path = testFilesUtils->flows_model();
//   MutantAnalyzer mutantAnalyzer(original_file_path);
//   auto number_of_mutants = 5;
//   mutantAnalyzer.createMutants(number_of_mutants);
//   auto mutants = mutantAnalyzer.getMutantFilePaths();
//   ASSERT_EQ(mutants.size(), number_of_mutants);
//   // Assert that all mutants are created
//   for (auto mutant : mutants) {
//     std::ifstream file(mutant);
//     ASSERT_TRUE(file.good());
//     // Ensure that the program is different from the original
//     std::ifstream original_file(original_file_path);
//     std::string original_program((std::istreambuf_iterator<char>(original_file)), std::istreambuf_iterator<char>());
//     std::ifstream mutant_file(mutant);
//     std::string mutant_program((std::istreambuf_iterator<char>(mutant_file)), std::istreambuf_iterator<char>());
//     ASSERT_NE(original_program, mutant_program);
//     // Remove the file
//     std::filesystem::remove(mutant);
//   }
// }

// TEST_F(MutantHandlerTest, AnalyzeMutants)
// {
//   auto original_file_path = testFilesUtils->array_model_never();
//   auto mutant_file_path = testFilesUtils->array_mutant_never();
//   MutantAnalyzer mutantAnalyzer(original_file_path, {mutant_file_path});
//   mutantAnalyzer.analyzeMutants();
//   auto mutants = mutantAnalyzer.getMutantFilePaths();
//   for (auto mutant : mutants) {
//     std::ifstream file(mutant);
//     ASSERT_TRUE(file.good());
//     // Remove the file
//     std::filesystem::remove(mutant);
//   }
// }

// TEST_F(MutantHandlerTest, EnhanceSpecificationToKillMutants_1Mutant)
// {
//   ASSERT_TRUE(testEnhanceSpecification(testFilesUtils->array_model_original(), {testFilesUtils->array_model_mutant()}, 0));
// }

// TEST_F(MutantHandlerTest, EnhanceSpecificationToKillMutants_RandomMutants)
// {
//   ASSERT_TRUE(testEnhanceSpecification(testFilesUtils->array_model_original(), {testFilesUtils->array_model_mutant()}, 5));
// }

// TEST_F(MutantHandlerTest, EnhanceSpecificationToKillMutants_1MutantAlt)
// {
//   ASSERT_TRUE(testEnhanceSpecification(testFilesUtils->array_model_original(), {testFilesUtils->array_model_mutant_alt()}, 0));
// }

// TEST_F(MutantHandlerTest, EnhanceSpecificationToKillMutants_2Mutants)
// {
//   auto original_file_path = testFilesUtils->array_model_original();
//   auto mutant_file_path_1 = testFilesUtils->array_model_mutant();
//   auto mutant_file_path_2 = testFilesUtils->array_model_mutant_alt();
//   std::vector<std::string> mutants = {mutant_file_path_1, mutant_file_path_2};
//   ASSERT_TRUE(testEnhanceSpecification(original_file_path, mutants, 0));
// }

// TEST_F(MutantHandlerTest, EnhanceSpecificationToKillMutantsTrafficLight)
// {
//   ASSERT_TRUE(testEnhanceSpecification(testFilesUtils->trafficLight_model_original(),
//                                        {testFilesUtils->trafficLight_model_mutant()}, 5));
// }

// TEST_F(MutantHandlerTest, EnhanceSpecificationToKillMutantsTrafficLight_TwoModels)
// {

//   auto original_file_path = testFilesUtils->trafficLight_model_original();
//   auto mutant_file_path_1 = testFilesUtils->trafficLight_model_mutant();
//   auto mutant_file_path_2 = testFilesUtils->trafficLight_model_mutant_alt();
//   std::vector<std::string> mutants = {mutant_file_path_1, mutant_file_path_2};
//   ASSERT_TRUE(testEnhanceSpecification(original_file_path, mutants, 0));
// }

TEST_F(MutantHandlerTest, EnhanceSpecification_3Processes)
{
  auto original_file_path = testFilesUtils->process_model_original();
  auto mutant_file_path_1 = testFilesUtils->process_model_mutant();
  auto res = testEnhanceSpecification(original_file_path, {mutant_file_path_1}, 5);

  ASSERT_TRUE(res);

  ASSERT_TRUE(testEnhanceSpecification(testFilesUtils->process_model_original(), {testFilesUtils->process_model_mutant()}, 5));
}

// TEST_F(MutantHandlerTest, EnhanceSpecification_TwoTrafficLights)
// {
//   ASSERT_TRUE(testEnhanceSpecification(testFilesUtils->two_trafficLight_model_original(), {}, 5));
// }

// TEST_F(MutantHandlerTest, EnhanceSpecificationToKillMutantsFlows)
// {
//   ASSERT_TRUE(testEnhanceSpecification(testFilesUtils->flows_model_original(), {testFilesUtils->flows_model_mutant()}, 5));
// }

// Ask SAMI about this test - it cannot find the mType
// TEST_F(MutantHandlerTest, EnhanceSpecificationToKillMutantsStructure)
// {
//   ASSERT_TRUE(testEnhanceSpecification(testFilesUtils->structure_model_original(),
//   {testFilesUtils->structure_model_mutant()}, 5, 15));
// }

// TEST_F(MutantHandlerTest, EnhanceSpecification_Mutex)
// {
//   ASSERT_TRUE(testEnhanceSpecification(testFilesUtils->mutex_original(), {}, 5));
// }


// TEST_F(MutantHandlerTest, EnhanceSpecification_PetersonMutex)
// {
//   ASSERT_TRUE(testEnhanceSpecification(testFilesUtils->peterson_original(), {}, 5));
// }

// TEST_F(MutantHandlerTest, EnhanceSpecification_LeaderElection)
// {
//   ASSERT_TRUE(testEnhanceSpecification(testFilesUtils->leader_election_original(), {}, 5));
// }

// TEST_F(MutantHandlerTest, EnhanceSpecification_Dijkstra)
// {
//   ASSERT_TRUE(testEnhanceSpecification(testFilesUtils->dijkstra_original(), {}, 5));
// }

// TEST_F(MutantHandlerTest, EnhanceSpecificationToKillMutantsMinepump)
// {
//   ASSERT_TRUE(
//       testEnhanceSpecification(testFilesUtils->minepump_model_original(), {testFilesUtils->minepump_model_mutant()}, 5));
// }