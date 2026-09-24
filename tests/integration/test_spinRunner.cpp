#include <filesystem>
#include <gtest/gtest.h>
#include "../TestFilesUtils.hpp"
#include <daedalux/mutants/spinRunner.hpp>
#include <string>

// Define a fixture for the tests
class SpinRunnerTest : public ::testing::Test {
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
};

TEST_F(SpinRunnerTest, TrafficLightModelIsCorrect)
{
  auto model_file = testFilesUtils->trafficLight_model_original();
  auto state_var = std::make_shared<VariableFormula>("state");
  auto state_var_equal_yellow = std::make_shared<EqualsFormula>(state_var, std::make_shared<VariableFormula>("yellow"));
  auto state_var_equal_red = std::make_shared<EqualsFormula>(state_var, std::make_shared<VariableFormula>("red"));
  auto eventually_yellow = std::make_shared<FinallyFormula>(state_var_equal_yellow);
  auto next_eventually_yellow = std::make_shared<NextFormula>(eventually_yellow);
  auto red_implies_next_eventually_yellow = std::make_shared<ImpliesFormula>(state_var_equal_red, next_eventually_yellow);
  auto ltl_formula = std::make_shared<GloballyFormula>(red_implies_next_eventually_yellow);
  auto result = spinRunner::check(model_file, ltl_formula);
  ASSERT_TRUE(result);
}

TEST_F(SpinRunnerTest, ModelIsCorrect)
{
  auto model_file = testFilesUtils->array_model();
  auto array_3 = std::make_shared<VariableFormula>("array[3]");
  auto array_equal_3 = std::make_shared<EqualsFormula>(array_3, std::make_shared<NumberConstant>(3));
  auto ltl_formula = std::make_shared<FinallyFormula>(array_equal_3);
  auto result = spinRunner::check(model_file, ltl_formula);
  ASSERT_TRUE(result);
}

TEST_F(SpinRunnerTest, ModelIsIncorrect)
{
  auto model_file = testFilesUtils->array_model();
  auto array_3 = std::make_shared<VariableFormula>("array[3]");
  auto array_equal_3 = std::make_shared<EqualsFormula>(array_3, std::make_shared<NumberConstant>(100));
  auto ltl_formula = std::make_shared<FinallyFormula>(array_equal_3);
  auto result = spinRunner::check(model_file, ltl_formula);
  ASSERT_FALSE(result);
}