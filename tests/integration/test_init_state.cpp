#include <daedalux/feature/tvl.hpp>
#include <daedalux/promela/parser/promela_loader.hpp>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

#include <daedalux/core/semantic.hpp>
#include <daedalux/algorithm/explore.hpp>
#include <daedalux/core/automata.hpp>


// Define a fixture for the tests
class stateInit : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override
  {
    // Common teardown code that will be called after each test
  }
  std::string helloWorld = "typedef S { int i; bool b = true };\n\
                    S s;\n\
                    active proctype test(){\n\
                     int i = 1; bool b; byte y[2]\n\
                     S s; s.i = 1;\n\
                    }";

  std::string helloChan = "chan c = [1] of {byte};\n\
                          chan d = [1] of {bool};\n\
                          chan cd = [2] of {byte, bool};\n\
                    active proctype test(){\n\
                     int i = 1; bool b;\n\
                     c!1; d!true; cd!1, true;\n\
                    }";

  std::string complexStruct;
};


TEST_F(stateInit, InitStateHelloWorld)
{
  const TVL * tvl = nullptr;
  auto original_loader = std::make_unique<promela_loader>(helloWorld, tvl);
  auto originalFSM = original_loader->getAutomata();
  // Create the initial state for both automata
  auto state = initState::createInitState(originalFSM.get(), tvl);

  auto s_global = state->getVariable("s");
  ASSERT_EQ(state->getValue<primitiveVariable*>("s.i"), 0);
  ASSERT_EQ(state->getValue<boolVar*>("s.b"), true);
  ASSERT_EQ(s_global->getValue<primitiveVariable*>("i"), 0);
  ASSERT_EQ(s_global->getValue<boolVar*>("b"), true);

  ASSERT_EQ(state->getValue<primitiveVariable*>("test.i"), 1);
  ASSERT_EQ(state->getValue<boolVar*>("test.b"), false);
  ASSERT_EQ(state->getValue<primitiveVariable*>("test.s.i"), 0);
  ASSERT_EQ(state->getValue<primitiveVariable*>("test.s.b"), true);
  ASSERT_EQ(state->getValue<primitiveVariable*>("test.y[0]"), 0);
  ASSERT_EQ(state->getValue<primitiveVariable*>("test.y[1]"), 0);

  auto proc = state->getVariable("test");
  ASSERT_EQ(dynamic_cast<process*>(proc)->getLocation(), 5);

  ASSERT_EQ(proc->getValue<primitiveVariable*>("i"), 1);
  ASSERT_EQ(proc->getValue<boolVar*>("b"), false);
  ASSERT_EQ(proc->getValue<primitiveVariable*>("s.i"), 0);
  ASSERT_EQ(proc->getValue<boolVar*>("s.b"), true);

  auto s_local = proc->getVariable("s");
  ASSERT_NE(s_global, s_local);
  ASSERT_EQ(s_local->getValue<primitiveVariable*>("i"), 0);
  ASSERT_EQ(s_local->getValue<boolVar*>("b"), true);
}

// TEST_F(stateInit, InitStateHelloWorldChan)
// {
//   const TVL * tvl = nullptr;
//   auto original_loader = std::make_unique<promela_loader>(helloChan, tvl);
//   auto originalFSM = original_loader->getAutomata();
//   // Create the initial state for both automata
//   auto state = initState::createInitState(originalFSM.get(), tvl);

//   auto c = state->getVariable("c");
//   ASSERT_EQ(c->getValue<primitiveVariable*>("c[0]"), 0);
//   ASSERT_EQ(dynamic_cast<channel*>(c)->len(), 1);

//   auto d = state->getVariable("d");
//   ASSERT_EQ(d->getValue<boolVar*>("d[0]"), false);
//   ASSERT_EQ(dynamic_cast<channel*>(d)->len(), 1);

//   auto cd = state->getVariable("cd");
//   ASSERT_EQ(cd->getValue<primitiveVariable*>("cd[0]"), 0);
//   ASSERT_EQ(cd->getValue<boolVar*>("cd[1]"), false);
//   ASSERT_EQ(dynamic_cast<channel*>(cd)->len(), 2);
// }

