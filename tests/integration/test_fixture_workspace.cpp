#include "../TestWorkspace.hpp"

#include <daedalux/core/logic/ltl.hpp>
#include <gtest/gtest.h>

#include <fstream>
#include <iterator>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {
std::string readFile(const fs::path & path)
{
  std::ifstream input(path);
  if (!input)
    throw std::runtime_error("Could not read " + path.string());
  return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
} // namespace

TEST(FixtureWorkspaceTest, ClaimRewritesLeaveSharedInputsUnchanged)
{
  const fs::path inputs = fs::canonical(fs::path(DAEDALUX_TEST_FIXTURES_DIR) / "test_files");
  std::vector<std::pair<fs::path, std::string>> originals;
  for (const auto & entry : fs::recursive_directory_iterator(inputs)) {
    if (entry.is_regular_file())
      originals.emplace_back(fs::relative(entry.path(), inputs), readFile(entry.path()));
  }

  {
    TestWorkspace workspace;
    for (const auto & [relative, contents] : originals) {
      if (relative.extension() == ".pml")
        LTLClaimsProcessor::removeClaimFromFile((workspace.path() / "test_files" / relative).string());
    }
  }

  for (const auto & [relative, contents] : originals)
    EXPECT_EQ(readFile(inputs / relative), contents) << relative;
}

TEST(FixtureWorkspaceTest, ConsecutiveCasesStartFreshAndCleanUpOutputs)
{
  const auto previous = fs::current_path();
  const auto original = readFile(fs::path(DAEDALUX_TEST_FIXTURES_DIR) / "test_files/mutants/flows/flows.pml");
  fs::path first;
  {
    TestWorkspace workspace;
    first = workspace.path();
    std::ofstream("test_files/mutants/flows/flows.pml", std::ios::trunc) << "changed";
    fs::create_directory("test_files/mutants/flows/flows_mutants");
    std::ofstream("test_files/mutants/flows/flows_mutants/mutant_1.pml") << "mutant";
    std::ofstream("incidental-output.txt") << "output";
  }
  EXPECT_EQ(fs::current_path(), previous);
  EXPECT_FALSE(fs::exists(first));
  {
    TestWorkspace workspace;
    EXPECT_NE(workspace.path(), first);
    EXPECT_EQ(readFile("test_files/mutants/flows/flows.pml"), original);
    EXPECT_FALSE(fs::exists("test_files/mutants/flows/flows_mutants"));
    EXPECT_FALSE(fs::exists("incidental-output.txt"));
  }
  EXPECT_EQ(fs::current_path(), previous);
}

TEST(FixtureWorkspaceTest, ConcurrentProcessesHavePrivateFixturesAndMutantFolders)
{
  int start[2];
  ASSERT_EQ(pipe(start), 0);
  std::vector<pid_t> children;
  for (int index = 0; index < 8; ++index) {
    const auto child = fork();
    if (child == 0) {
      close(start[1]);
      char signal;
      if (read(start[0], &signal, 1) != 1)
        _exit(1);
      close(start[0]);
      int result = 0;
      try {
        TestWorkspace workspace;
        const auto marker = std::to_string(getpid());
        const fs::path fixture = "test_files/mutants/flows/flows.pml";
        const fs::path mutants = "test_files/mutants/flows/flows_mutants";
        fs::create_directory(mutants);
        for (int iteration = 0; iteration < 100; ++iteration) {
          std::ofstream(fixture, std::ios::trunc) << marker;
          std::ofstream(mutants / "mutant_1.pml", std::ios::trunc) << marker;
          usleep(1000);
          if (readFile(fixture) != marker || readFile(mutants / "mutant_1.pml") != marker) {
            result = 1;
            break;
          }
        }
      }
      catch (...) {
        result = 1;
      }
      _exit(result);
    }
    if (child < 0) {
      ADD_FAILURE() << "fork failed";
      break;
    }
    children.push_back(child);
  }
  close(start[0]);
  for (size_t index = 0; index < children.size(); ++index)
    EXPECT_EQ(write(start[1], "s", 1), 1);
  close(start[1]);
  for (const auto child : children) {
    int status = 0;
    ASSERT_EQ(waitpid(child, &status, 0), child);
    EXPECT_TRUE(WIFEXITED(status));
    if (WIFEXITED(status))
      EXPECT_EQ(WEXITSTATUS(status), 0);
  }
}
