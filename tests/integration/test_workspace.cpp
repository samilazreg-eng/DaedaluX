#include "../support/TestEnvironment.hpp"
#include "../support/TestPaths.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <sys/wait.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace {

// Leaves an output and a private copy. Run by two cases: whichever comes second in a process must not see them.
void expectEmptyWorkspaceThenDirtyIt()
{
  EXPECT_TRUE(fs::equivalent(fs::current_path(), testWorkspace()));
  EXPECT_TRUE(fs::is_empty(testWorkspace())) << "the previous case left its files behind";

  std::ofstream("output.txt") << "output";
  privateTestFile("basic/array.pml");
}

} // namespace

TEST(TestWorkspaceTest, CaseStartsInAnEmptyWorkspace) { expectEmptyWorkspaceThenDirtyIt(); }

TEST(TestWorkspaceTest, NextCaseStartsInAnEmptyWorkspaceToo) { expectEmptyWorkspaceThenDirtyIt(); }

// The loader and the parser leave through exit(1) on errors, which runs static destructors, the workspace's included.
// A child doing so, such as the one of EXPECT_EXIT, must not remove the workspace its parent is still using.
TEST(TestWorkspaceTest, ForkedChildLeavingThroughExitKeepsTheWorkspace)
{
  pid_t pid = fork();
  ASSERT_NE(pid, -1);
  if (pid == 0)
    exit(0);
  int status = 0;
  waitpid(pid, &status, 0);

  EXPECT_TRUE(fs::exists(testWorkspace()));
}

TEST(TestPathsTest, PrivateFileIsACopyInTheWorkspace)
{
  const fs::path shared = sharedTestFile("basic/array.pml");
  const fs::path copy = privateTestFile("basic/array.pml");
  ASSERT_EQ(copy, testWorkspace() / "data/basic/array.pml"); // the write below must not reach the shared file
  const auto size = fs::file_size(shared);
  EXPECT_EQ(fs::file_size(copy), size);

  std::ofstream(copy) << "rewritten";

  EXPECT_EQ(fs::file_size(shared), size);
  EXPECT_EQ(fs::file_size(privateTestFile("basic/array.pml")), 9u) << "the second use made a new copy";
}
