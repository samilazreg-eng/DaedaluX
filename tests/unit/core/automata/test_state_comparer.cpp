#include <daedalux/core/semantic/variable/state/stateVar.hpp>
#include <daedalux/formulas/formulaCreator.hpp>
#include <daedalux/algorithm/utils/stateComparer.hpp>
#include <daedalux/promela/parser/promela_loader.hpp>
#include <daedalux/promela/semantic/variable/state/initState.hpp>

#include "../../../support/TestFilesUtils.hpp"

#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

// Define a fixture for the tests
class StateComparerTest : public ::testing::Test {
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

TEST_F(StateComparerTest, sameStatesArrayModel)
{
  auto original = initState::createInitState(std::make_unique<promela_loader>(testFilesUtils->array_model_original())->getAutomata().get());
  auto mutant = initState::createInitState(std::make_unique<promela_loader>(testFilesUtils->array_model_mutant())->getAutomata().get());

  std::list<std::shared_ptr<state>> originalStates; originalStates.push_back(std::shared_ptr<state>(original));
  std::list<std::shared_ptr<state>> mutantStates; mutantStates.push_back(std::shared_ptr<state>(mutant));

  original->print();
  mutant->print();

  ASSERT_TRUE(StateComparer::sameStates(originalStates, mutantStates, true));
}