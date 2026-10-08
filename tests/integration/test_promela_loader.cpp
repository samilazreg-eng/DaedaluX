#include <daedalux/promela/parser/promela_loader.hpp>


#include "../support/TestPaths.hpp"
#include <gtest/gtest.h>
#include <memory>

namespace fs = std::filesystem;

// Define a fixture for the tests
class PromelaLoaderTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    // Common setup code that will be called before each test
  }

  void TearDown() override
  {
    // Common teardown code that will be called after each test
  }
};

// Test case for loading an invalid Promela file
// TEST_F(PromelaLoaderTest, LoadInvalidPromelaFile)
// {
//   // Load promela file
//   std::string file_name = "invalid_file.txt";
//   const TVL * tvl = nullptr;
//   // Call the function under test and expect it to exit with code 1
//   EXPECT_EXIT(std::make_unique<promela_loader>(file_name, tvl), ::testing::ExitedWithCode(1),
//               "The model file must have the extension .pml.");
// }

// Test case for loading a valid Promela file
TEST_F(PromelaLoaderTest, LoadValidPromelaFileFlows)
{
  std::string file_path = sharedTestFile("basic/flows.pml");
  const TVL * tvl = nullptr;
  auto loader = std::make_unique<promela_loader>(file_path, tvl);

  auto automata = loader->getAutomata();

  EXPECT_EQ(automata->getNodes().size(), 9);
  EXPECT_EQ(automata->getTransitions().size(), 16);

  auto initialStates = automata->getInitNodes();
  EXPECT_EQ(initialStates.size(), 2);
}

// Test case for loading a valid Promela file
TEST_F(PromelaLoaderTest, LoadValidPromelaFileArray)
{
  std::string file_path = sharedTestFile("basic/array.pml");
  const TVL * tvl = nullptr;
  auto loader = std::make_unique<promela_loader>(file_path, tvl);

  auto automata = loader->getAutomata();

  EXPECT_EQ(automata->getNodes().size(), 5);
  EXPECT_EQ(automata->getTransitions().size(), 6);

  auto initialStates = automata->getInitNodes();
  EXPECT_EQ(initialStates.size(), 1);
}

// Test case for loading a valid Promela file
// TEST_F(PromelaLoaderTest, LoadMinePumpPromelaFile)
// {

//   std::string current_directory = fs::current_path();
//   std::string file_name = "/models/minepump/original.pml";
//   std::string file_path = current_directory + file_name;
//   const TVL * tvl = nullptr;
//   auto loader = std::make_unique<promela_loader>(file_path, tvl);

//   auto automata = loader->getAutomata();

//   EXPECT_EQ(automata->getNodes().size(), 46);
//   EXPECT_EQ(automata->getTransitions().size(), 67);

//   auto initialStates = automata->getInitNodes();
//   EXPECT_EQ(initialStates.size(), 5);
// }

// // Test case for loading Windows Promela file
// TEST_F(PromelaLoaderTest, LoadWindowsPromelaFile)
// {
//   std::string current_directory = fs::current_path();
//   std::string file_name = "/models/windows/original.pml";
//   std::string file_path = current_directory + file_name;
//   const TVL * tvl = nullptr;
//   auto loader = std::make_unique<promela_loader>(file_path, tvl);

//   auto automata = loader->getAutomata();

//   int numNodes = automata->getNodes().size();
//   int numTransitions = automata->getTransitions().size();

//   EXPECT_EQ(numNodes, 35);
//   EXPECT_EQ(numTransitions, 46);

//   auto initialStates = automata->getInitNodes();
//   EXPECT_EQ(initialStates.size(), 2);
// }