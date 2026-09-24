#include "../TestFilesUtils.hpp"
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

#include <daedalux/algorithm.hpp>


// Define a fixture for the tests
class SimilarityTest : public ::testing::Test {
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


  successorTree KSuccessors(std::string file_path, int k)
  {
    const TVL * tvl = nullptr;
    auto loader = std::make_unique<promela_loader>(file_path, tvl);
    auto myFSM = loader->getAutomata().get();
    auto current_state = initState::createInitState(myFSM, tvl);
    return fsmExplorer::kSuccessors(current_state, k);
  }

  state * kSuccessor(state * st, int k)
  {
    while (k > 0) {
      st = st->Post().front();
      k--;
    }
    return st;
  }
};

TEST_F(SimilarityTest, DistinctState_EmptyList)
{
  printf("Comparing distinct states of an empty list\n");
  const TVL * tvl = nullptr;
  auto file_path = testFilesUtils->array_model();
  auto loader = std::make_unique<promela_loader>(file_path, tvl);
  auto originalFSM = loader->getAutomata().get();
  // Create the initial state for both automata
  auto current_state_original = initState::createInitState(originalFSM, tvl);
  auto post_states_original = std::list<state *>{current_state_original};
  std::list<state *> post_states_mutant;
  ASSERT_EQ(post_states_original.empty(), false);
  ASSERT_EQ(post_states_mutant.empty(), true);
  auto different_states = StateComparer::distinct_states(post_states_original, post_states_mutant);
  ASSERT_EQ(different_states, post_states_original);
}

// TEST_F(SimilarityTest, DistinctStates_DifferentFSM)
// {
//   auto file_path1 = testFilesUtils->array_model();
//   auto file_path2 = testFilesUtils->flows_model();
//   const TVL * tvl = nullptr;
//   auto loader1 = std::make_unique<promela_loader>(file_path1, tvl);
//   auto loader2 = std::make_unique<promela_loader>(file_path2, tvl);
//   auto myFSM = loader1->getAutomata().get();
//   auto mutant = loader2->getAutomata().get();
//   // Create the initial state for both automata
//   auto current_state_original = initState::createInitState(myFSM, tvl);
//   auto current_state_mutant = initState::createInitState(mutant, tvl);
//   auto post_states_original = current_state_original->Post();
//   auto post_states_mutant = current_state_mutant->Post();
//   ASSERT_EQ(post_states_original.empty(), false);
//   ASSERT_EQ(post_states_mutant.empty(), false);
//   auto different_states = StateComparer::distinct_states(post_states_original, post_states_mutant);
//   ASSERT_EQ(different_states.empty(), false);
//   ASSERT_EQ(different_states.size(), post_states_original.size());
// }

TEST_F(SimilarityTest, DistinctStates_SameFSM)
{
  printf("Comparing distinct states of the same FSM\n");
  // Create the initial state for both automata
  const TVL * tvl = nullptr;
  auto loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto myFSM = loader->getAutomata().get();
  auto current_state_original = initState::createInitState(myFSM, tvl);
  auto post_states_original = current_state_original->Post();
  auto post_states_mutant = current_state_original->Post();
  ASSERT_EQ(post_states_original.empty(), false);
  ASSERT_EQ(post_states_mutant.empty(), false);
  ASSERT_EQ(post_states_original.size(), post_states_mutant.size());
  auto different_states = StateComparer::distinct_states(post_states_original, post_states_mutant);
  ASSERT_EQ(different_states.empty(), true);
}

TEST_F(SimilarityTest, SameStateDelta_ShouldBe0)
{
  const TVL * tvl = nullptr;
  auto loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto myFSM = loader->getAutomata().get();
  auto current_state = initState::createInitState(myFSM, tvl);
  bool considerInternalVariables = true;
  auto delta = current_state->delta(current_state, considerInternalVariables);
  auto expected = 0.0;
  ASSERT_EQ(delta - expected < 0.0001, true);
}

TEST_F(SimilarityTest, DifferentStateDelta_ShouldNotBe0)
{
  const TVL * tvl = nullptr;
  auto loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto myFSM = loader->getAutomata().get();
  auto current_state = initState::createInitState(myFSM, tvl);
  auto post_state = current_state->Post().front()->Post().front()->Post().front()->Post().front()->Post().front();
  bool considerInternalVariables = false;
  auto delta = current_state->delta(post_state, considerInternalVariables);
  float expected = 0.125;
  ASSERT_TRUE(expected - delta < 0.00001);
  auto post_post_state = post_state->Post().front()->Post().front()->Post().front()->Post().front()->Post().front();
  delta = current_state->delta(post_post_state, considerInternalVariables);
  expected = 0.291666687;
  ASSERT_TRUE(expected - delta < 0.00001);
}

TEST_F(SimilarityTest, MostSimilarStateEmptyList)
{
  const TVL * tvl = nullptr;
  auto loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto myFSM = loader->getAutomata().get();
  auto current_state = initState::createInitState(myFSM, tvl);
  std::list<state *> post_states;
  auto most_similar = StateComparer::most_similar_state(current_state, post_states);
  ASSERT_TRUE(most_similar == nullptr);
}

TEST_F(SimilarityTest, MinepumpMostSimilarStateOneElement)
{
  const TVL * tvl = nullptr;
  auto loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto myFSM = loader->getAutomata().get();
  auto current_state = initState::createInitState(myFSM, tvl);
  auto post_state = current_state->Post().front();
  std::list<state *> post_states = {post_state};
  auto most_similar = StateComparer::most_similar_state(current_state, post_states);
  ASSERT_TRUE(most_similar != nullptr);
  ASSERT_TRUE(most_similar == post_state);
}

// TEST_F(SimilarityTest, FlowMostSimilarStateOfSameState)
// {
//   const TVL * tvl = nullptr;
//   auto file_path = current_path + file2;
//   auto loader = std::make_unique<promela_loader>(file_path, tvl);
//   auto myFSM = loader->getAutomata().get();
//   auto current_state = initState::createInitState(myFSM, tvl);
//   auto post_states = current_state->Post();
//   post_states.push_back(current_state);
//   ASSERT_TRUE(post_states.size() > 0);
//   auto most_similar = most_similar_state(current_state, post_states);
//   ASSERT_TRUE(most_similar != nullptr);
//   ASSERT_EQ(most_similar, current_state);
// }

TEST_F(SimilarityTest, MostSimilarStateOneElement)
{
  const TVL * tvl = nullptr;
  auto loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto myFSM = loader->getAutomata().get();
  auto current_state = initState::createInitState(myFSM, tvl);
  auto post_state = current_state->Post().front();
  std::list<state *> post_states = {post_state};
  auto most_similar = StateComparer::most_similar_state(current_state, post_states);
  ASSERT_TRUE(most_similar == post_state);
}

TEST_F(SimilarityTest, MostSimilarStateOfSameState)
{
  const TVL * tvl = nullptr;
  auto loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto myFSM = loader->getAutomata().get();
  auto current_state = initState::createInitState(myFSM, tvl);
  auto post_states = current_state->Post();
  post_states.push_back(current_state);
  ASSERT_TRUE(post_states.size() > 0);
  auto most_similar = StateComparer::most_similar_state(current_state, post_states);
  ASSERT_TRUE(most_similar != nullptr);
  ASSERT_TRUE(most_similar->isSame(current_state, true));
}

TEST_F(SimilarityTest, DistinctStates_ShouldReturnTheFirstList)
{
  const TVL * tvl = nullptr;
  auto loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto myFSM = loader->getAutomata().get();
  auto current_state = initState::createInitState(myFSM, tvl);
  auto post_state_front = current_state->Post().front()->Post().front()->Post().front();
  std::list<state *> post_states_1 = {post_state_front};
  auto post_state_back = post_state_front->Post().front()->Post().front()->Post().front()->Post().front();
  // The two states are different
  ASSERT_FALSE(post_state_back->isSame(post_state_front, true));
  std::list<state *> post_states_2 = {post_state_back};
  auto distinct_States = StateComparer::distinct_states(post_states_1, post_states_2);
  ASSERT_TRUE(distinct_States.size() == post_states_1.size());
  ASSERT_TRUE(distinct_States.front() == post_state_front);
}

TEST_F(SimilarityTest, DistinctStates_IdenticalLists_ShouldReturnEmptyList)
{
  const TVL * tvl = nullptr;
  auto loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto myFSM = loader->getAutomata().get();
  auto current_state = initState::createInitState(myFSM, tvl);
  auto post_state = current_state->Post().front();
  std::list<state *> post_states_1 = {post_state};
  auto distinct_States = StateComparer::distinct_states(post_states_1, post_states_1);
  ASSERT_TRUE(distinct_States.empty());
}

TEST_F(SimilarityTest, DistinctStates_OverlappingLists)
{
  const TVL * tvl = nullptr;
  auto loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto myFSM = loader->getAutomata().get();
  auto current_state = initState::createInitState(myFSM, tvl);
  auto post_state_1 = kSuccessor(current_state, 5);
  std::list<state *> post_states_1 = {post_state_1, current_state};
  auto post_state_2 = kSuccessor(current_state, 10);
  bool considerInternalVariables = false;
  // The two states are different
  ASSERT_FALSE(post_state_1->isSame(post_state_2, considerInternalVariables));
  std::list<state *> post_states_2 = {post_state_2, current_state};
  auto distinct_States = StateComparer::distinct_states(post_states_1, post_states_2);
  ASSERT_TRUE(distinct_States.size() == 1);
  ASSERT_TRUE(distinct_States.front() == post_state_1);
}

TEST_F(SimilarityTest, KSuccessorsArray)
{
  auto file_path = testFilesUtils->array_model();
  auto k = 5;
  auto result = KSuccessors(file_path, k);
  ASSERT_EQ(result.length(), 5);
}

TEST_F(SimilarityTest, KSuccessorsFlows)
{
  auto file_path = testFilesUtils->flows_model();
  auto k = 5;
  auto result = KSuccessors(file_path, k);
  ASSERT_EQ(result.length(), 5);
}

TEST_F(SimilarityTest, KSuccessorsArray_Difference_ShouldBeEmpty)
{
  auto file_path = testFilesUtils->array_model();
  auto k = 5;
  auto result_1 = KSuccessors(file_path, k);
  auto result_2 = KSuccessors(file_path, k);
  ASSERT_EQ(result_1.length(), result_2.length());
  auto result = StateComparer::compareKSuccessors(result_1, result_2);
  ASSERT_TRUE(result.getMutantOnly().empty());
  ASSERT_TRUE(result.getOriginalOnly().empty());
  ASSERT_TRUE(!result.getCommon().empty());
}

// TEST_F(SimilarityTest, KSuccessorsArrayMutant_Difference_ShouldBeNonEmpty)
// {
//   auto k = 11;
//   auto result_1 = KSuccessors(testFilesUtils->array_model_original(), k);
//   auto result_2 = KSuccessors(testFilesUtils->array_model_mutant(), k);
//   ASSERT_EQ(result_1.size(), result_2.size());
//   auto result = StateComparer::compareKSuccessors(result_1, result_2);
//   bool some_difference = false;
//   for (auto & [key, value] : result.getMutantOnly()) {
//     if (!value.empty()) {
//       some_difference = true;
//       break;
//     }
//   }
//   for (auto & [key, value] : result.getOriginalOnly()) {
//     if (!value.empty()) {
//       some_difference = true;
//       break;
//     }
//   }
//   ASSERT_TRUE(some_difference);
// }