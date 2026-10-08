#include <daedalux/core/semantic.hpp>
#include <daedalux/formulas.hpp>
#include <daedalux/mutants.hpp>

#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

#include "../support/TestFilesUtils.hpp"

// Define a fixture for the tests
class ModelAnalyzerTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    // Common setup code that will be called before each test
    testFilesUtils = std::make_unique<TestFilesUtils>(sharedTestFile);
  }

  void TearDown() override
  {
    // Common teardown code that will be called after each test
  }

  std::unique_ptr<TestFilesUtils> testFilesUtils;
};

TEST_F(ModelAnalyzerTest, model_analysis_array)
{
  auto original_file_path = testFilesUtils->array_model();
  ModelAnalyzer modelAnalyzer(original_file_path);
  modelAnalyzer.analyzeVariableUpdates();
}

TEST_F(ModelAnalyzerTest, model_analysis_two_trafficLights)
{
  auto original_file_path = testFilesUtils->two_trafficLight_model_original();
  ModelAnalyzer modelAnalyzer(original_file_path);
  modelAnalyzer.analyzeVariableUpdates();
}

TEST_F(ModelAnalyzerTest, model_analysis_trafficLight)
{
  auto original_file_path = testFilesUtils->trafficLight_model_original();
  ModelAnalyzer modelAnalyzer(original_file_path);
  modelAnalyzer.analyzeVariableUpdates();
}

// TEST_F(ModelAnalyzerTest, model_analysis_flows_model)
// {
//   auto original_file_path = testFilesUtils->flows_model_original();
//   ModelAnalyzer modelAnalyzer(original_file_path);
//   modelAnalyzer.analyzeVariableUpdates();
// }

// TEST_F(ModelAnalyzerTest, model_analysis_peterson_model)
// {
//   auto original_file_path = testFilesUtils->minepump_model_original();
//   ModelAnalyzer modelAnalyzer(original_file_path);
//   modelAnalyzer.analyzeVariableUpdates();
// }