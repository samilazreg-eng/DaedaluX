#include <daedalux/algorithm/explore.hpp>
#include <daedalux/core/semantic/variable/state/composite.hpp>
#include <daedalux/promela/semantic/variable/state/initState.hpp>
#include <daedalux/core/semantic/variable/transition/compositeTransition.hpp>
#include <daedalux/promela/parser/promela_loader.hpp>
#include <daedalux/visualizer/trace.hpp>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

class TraceTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    // Common setup code that will be called before each test
    myTrace = std::make_unique<trace>();
  }
  void TearDown() override
  {
    // Common teardown code that will be called after each test
  }
  std::unique_ptr<trace> myTrace;
};

// Test case for adding transitions
TEST_F(TraceTest, AddTransition)
{
  std::list<transition *> transList = std::list<transition *>();
  std::shared_ptr<state> state = std::make_shared<composite>("test_variable");
  std::shared_ptr<transition> t = std::make_shared<compTransition>(state.get(), transList);
  myTrace->addTransition(t);
  ASSERT_EQ(myTrace->getTransitions().size(), 1);
}

// Test case for adding states
TEST_F(TraceTest, AddState)
{
  std::shared_ptr<state> state = std::make_shared<composite>("test_variable");
  myTrace->addState(state);
  ASSERT_EQ(myTrace->size(), 1);
}

// Test case for adding trace
TEST_F(TraceTest, AddTrace)
{
  std::unique_ptr<trace> other = std::make_unique<trace>();
  std::list<transition *> transList = std::list<transition *>();
  std::shared_ptr<state> state = std::make_shared<composite>("test_variable");
  std::shared_ptr<transition> t = std::make_shared<compTransition>(state.get(), transList);
  other->addTransition(t);
  other->addState(state);
  myTrace->addTrace(other.get());
  ASSERT_EQ(myTrace->size(), 1);
  ASSERT_EQ(myTrace->getStates().size(), 1);
}

// Test case for equality operator
TEST_F(TraceTest, EqualityOperator)
{
  trace t1 = trace();
  trace t2 = trace();
  ASSERT_TRUE(t1 == t2);
}

TEST_F(TraceTest, InequalityOperatorAdvanced)
{
  trace t1 = trace();
  trace t2 = trace();

  std::list<transition *> transList = std::list<transition *>();
  std::shared_ptr<state> state = std::make_shared<composite>("test_variable");
  std::shared_ptr<transition> t = std::make_shared<compTransition>(state.get(), transList);
  t1.addTransition(t);
  t2.addState(state);
  ASSERT_TRUE(t1 != t2);
}

// Test case for inequality operator
TEST_F(TraceTest, InequalityOperator)
{
  trace t1 = trace();
  trace t2 = trace();
  ASSERT_FALSE(t1 != t2);
}

// TEST_F(TraceTest, EqualityOperatorAdvanced)
// {
//   std::unique_ptr<trace> otherTrace = std::make_unique<trace>();
//   auto file = "/test_files/basic/array.pml";
//   std::string current_path = std::filesystem::current_path();
//   const TVL * tvl = nullptr;
//   auto file_path = current_path + file;
//   printf("Loading file: %s\n", file_path.c_str());
//   auto loader = std::make_unique<promela_loader>(file_path, tvl);
//   auto FSM = loader->getAutomata();
//   printf("Created automata\n");
//   auto current_state = initState::createInitState(FSM.get(), tvl);
//   auto next_state = current_state->Post().front();
//   transition * current_trans = next_state->getOrigin()->deepCopy();
//   std::shared_ptr<state> curent_state_smart(current_state);
//   std::shared_ptr<state> next_state_smart(next_state);
//   std::shared_ptr<transition> current_trans_smart(current_trans);
//   myTrace->addState(curent_state_smart);
//   myTrace->addState(next_state_smart);
//   myTrace->addTransition(current_trans_smart);
//   otherTrace->addState(curent_state_smart);
//   otherTrace->addState(next_state_smart);
//   otherTrace->addTransition(current_trans_smart);
//   ASSERT_TRUE(myTrace.get() == otherTrace.get());
// }

// TEST_F(TraceTest, InEqualityOperatorAdvanced_Minepump)
// {
//   std::unique_ptr<trace> otherTrace = std::make_unique<trace>();
//   auto file = "/models/minepump/original.pml";
//   std::string current_path = std::filesystem::current_path();
//   const TVL * tvl = nullptr;
//   auto file_path = current_path + file;
//   auto loader = std::make_unique<promela_loader>(file_path, tvl);
//   auto FSM = loader->getAutomata();
//   auto current_state = initState::createInitState(FSM.get(), tvl);
//   auto next_state = current_state->Post().front();
//   // transition * current_trans = const_cast<transition *>(next_state->getOrigin());
//   std::shared_ptr<state> curent_state_smart(current_state);
//   std::shared_ptr<state> next_state_smart(next_state);
//   // std::shared_ptr<transition> current_trans_smart(current_trans);
//   myTrace->addState(curent_state_smart);
//   myTrace->addState(next_state_smart);
//   // myTrace->addTransition(current_trans_smart);
//   otherTrace->addState(curent_state_smart);
//   otherTrace->addState(next_state_smart);
//   ASSERT_TRUE(myTrace.get() != otherTrace.get());
// }

// TEST_F(TraceTest, PrintCSV)
// {
//   auto file = "/test_files/basic/flows.pml";
//   std::string current_path = std::filesystem::current_path();
//   const TVL * tvl = nullptr;
//   auto file_path = current_path + file;
//   auto loader = std::make_unique<promela_loader>(file_path, tvl);
//   auto FSM = loader->getAutomata();
//   auto current_state = initState::createInitState(FSM.get(), tvl);
//   auto next_state = current_state->Post().front();
//   transition * current_trans = next_state->getOrigin()->deepCopy();
//   std::shared_ptr<state> curent_state_smart(current_state);
//   std::shared_ptr<state> next_state_smart(next_state);
//   std::shared_ptr<transition> current_trans_smart(current_trans);
//   myTrace->addState(curent_state_smart);
//   myTrace->addState(next_state_smart);
//   myTrace->addTransition(current_trans_smart);
//   std::stringstream ss;
//   myTrace->printCSV(ss);
//   // Write the expected output
//   std::string expected = "sys..s.a,sys..s.b,sys..s.c,sys..s.d,sys..I,sys..J,prob,\nfalse,false,false,false,12,31,1,\ntrue,"
//                          "false,false,false,13,31,1,\n";
//   ASSERT_EQ(ss.str(), expected);
// }


// TEST_F(TraceTest, TraceOfFlowLong)
// {
//   auto file = "/test_files/basic/flows.pml";
//   std::string current_path = std::filesystem::current_path();
//   const TVL * tvl = nullptr;
//   auto length = 20;
//   int original_length = length;
//   auto file_path = current_path + file;
//   auto loader = std::make_unique<promela_loader>(file_path, tvl);
//   auto FSM = loader->getAutomata();
//   auto current_state = initState::createInitState(FSM.get(), tvl);
//   std::shared_ptr<state> curent_state_smart(current_state);
//   myTrace->addState(curent_state_smart);
//   while (length > 0) {
//     auto next_state = current_state->Post().front();
//     transition * current_trans = next_state->getOrigin()->deepCopy();
//     std::shared_ptr<state> next_state_smart(next_state);
//     std::shared_ptr<transition> current_trans_smart(current_trans);
//     myTrace->addState(next_state_smart);
//     myTrace->addTransition(current_trans_smart);
//     current_state = next_state;
//     length--;
//   }
//   ASSERT_EQ(myTrace->size(), original_length);
//   ASSERT_EQ(myTrace->getStates().size(), original_length + 1);
//   ASSERT_EQ(myTrace->getTransitions().size(), original_length);
// }

// TEST_F(TraceTest, TraceOfMinepump)
// {
//   auto file = "/models/minepump/original.pml";
//   std::string current_path = std::filesystem::current_path();
//   const TVL * tvl = nullptr;
//   auto file_path = current_path + file;
//   auto loader = std::make_unique<promela_loader>(file_path, tvl);
//   auto FSM = loader->getAutomata();
//   auto current_state = initState::createInitState(FSM.get(), tvl);
//   std::shared_ptr<state> curent_state_smart(current_state);
//   myTrace->addState(curent_state_smart);
//   auto next_state = current_state->Post().front();
//   transition * current_trans = next_state->getOrigin()->deepCopy();
//   std::shared_ptr<state> next_state_smart(next_state);
//   std::shared_ptr<transition> current_trans_smart(current_trans);
//   myTrace->addState(next_state_smart);
//   myTrace->addTransition(current_trans_smart);
//   ASSERT_EQ(myTrace->size(), 1);
//   ASSERT_EQ(myTrace->getStates().size(), 2);
//   ASSERT_EQ(myTrace->getTransitions().size(), 1);
// }

// TEST_F(TraceTest, TraceOfMinepump_Long)
// {
//   auto file = "/models/minepump/original.pml";
//   std::string current_path = std::filesystem::current_path();
//   auto length = 10;
//   int original_length = length;
//   const TVL * tvl = nullptr;
//   auto file_path = current_path + file;
//   auto loader = std::make_unique<promela_loader>(file_path, tvl);
//   auto FSM = loader->getAutomata();
//   auto current_state = initState::createInitState(FSM.get(), tvl);
//   std::shared_ptr<state> curent_state_smart(current_state);
//   myTrace->addState(curent_state_smart);
//   printf("Created initial state\n");
//   while (length > 0) {
//     auto next_states = current_state->Post();
//     if (next_states.empty()) {
//       break;
//     }
//     auto next_state = next_states.front();
//     transition * current_trans = next_state->getOrigin()->deepCopy();
//     std::shared_ptr<state> next_state_smart(next_state);
//     std::shared_ptr<transition> current_trans_smart(current_trans);
//     myTrace->addState(next_state_smart);
//     myTrace->addTransition(current_trans_smart);
//     current_state = next_state;
//     printf("Added state and transition - missing %d\n", length);
//     length--;
//   }
//   myTrace->printCSV(std::cout);
//   ASSERT_EQ(myTrace->size(), original_length);
//   ASSERT_EQ(myTrace->getStates().size(), original_length + 1);
//   ASSERT_EQ(myTrace->getTransitions().size(), original_length);
// }