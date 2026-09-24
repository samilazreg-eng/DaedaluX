#include <filesystem>
#include <daedalux/feature/tvl.hpp>
#include <daedalux/promela/parser/promela_loader.hpp>
#include <daedalux/promela/semantic/variable/state/initState.hpp>
#include <daedalux/core/semantic/variable/state/composite.hpp>
#include <gtest/gtest.h>

class StateTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    // Common setup code for all tests goes here
    s = std::make_unique<composite>("testState");
  }

  void TearDown() override
  {
    // Common teardown code for all tests goes here
  }
  std::unique_ptr<state> s;
  std::string array_model = "/test_files/basic/array.pml";
  std::string flow_model = "/test_files/basic/flows.pml";
  std::string current_path = std::filesystem::current_path();
};

TEST_F(StateTest, DefaultConstructor)
{
  EXPECT_EQ(variable::Type::V_COMP_S, s->getType());
  EXPECT_EQ("testState", s->getFullName());
  EXPECT_EQ(1.0, s->getProbability());
  EXPECT_EQ(nullptr, s->getOrigin());
  EXPECT_EQ(0u, s->getErrorMask());
  EXPECT_TRUE(s->executables().empty());
  EXPECT_TRUE(s->hasDeadlock());
  EXPECT_EQ(0u, s->getErrorMask());
}

TEST_F(StateTest, CopyConstructor)
{
  // Create a state object to copy from
  auto originalState = new composite("test_variable");
  originalState->prob = 0.5;

  // Create a new state object using the copy constructor
  // auto copiedState = new composite(originalState);

  // Verify that the copied state has the same values as the original state
  // EXPECT_EQ(copiedState->prob, originalState->prob);
  // EXPECT_EQ(copiedState->origin, originalState->origin);
  // EXPECT_EQ(copiedState->errorMask, originalState->errorMask);

  // delete copiedState;
  // delete originalState;
}

TEST_F(StateTest, PostStateArray)
{
  const TVL * tvl = nullptr;
  auto file_path1 = current_path + array_model;
  auto loader = std::make_unique<promela_loader>(file_path1, tvl);
  auto myFSM = loader->getAutomata().get();
  auto current_state = initState::createInitState(myFSM, tvl);
  state * next_state = nullptr;
  bool new_state_found = false;
  int search_depth = 10;
  for (auto i = 0; i < search_depth; i++) {
    next_state = current_state->Post().front();
    if (!next_state->isSame(current_state, false)) {
      new_state_found = true;
      break;
    }
    current_state = next_state;
  }
  ASSERT_TRUE(new_state_found);
}

TEST_F(StateTest, PostStateFlow)
{
  const TVL * tvl = nullptr;
  auto file_path1 = current_path + flow_model;
  auto loader = std::make_unique<promela_loader>(file_path1, tvl);
  auto myFSM = loader->getAutomata().get();
  auto current_state = initState::createInitState(myFSM, tvl);
  state * next_state = nullptr;
  bool new_state_found = false;
  bool considerInternalVariables = false;
  int search_depth = 10;
  for (auto i = 0; i < search_depth; i++) {
    auto successors = current_state->Post();
    next_state = successors.front();
    if (!next_state->isSame(current_state, considerInternalVariables)) {
      new_state_found = true;
      break;
    }
    current_state = next_state;
  }
  ASSERT_TRUE(new_state_found);
}