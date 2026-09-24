#include <daedalux/promela/parser/promela_loader.hpp>

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace {

std::string modelWithProctype(const std::string & name)
{
  return "byte x = 0;\n"
         "active proctype " + name + "(){\n"
         "  x = 1;\n"
         "}\n";
}

// Exit status for a child process: 0 if every load parsed `model`, 2 if a load parsed another model.
int loadRepeatedly(const std::string & model, const std::string & expected, int rounds)
{
  for (int i = 0; i < rounds; ++i) {
    promela_loader loader(model);
    if (stmnt::string(loader.getProgram()).find(expected) == std::string::npos)
      return 2;
  }
  return 0;
}

// Runs the test body in a fresh directory and restores the working directory afterwards.
class ScopedWorkingDirectory {
public:
  ScopedWorkingDirectory() : previous(fs::current_path())
  {
    auto pattern = (fs::temp_directory_path() / "daedalux-test-XXXXXX").string();
    if (mkdtemp(pattern.data()) == nullptr)
      throw std::runtime_error("mkdtemp failed");
    path = pattern;
    fs::current_path(path);
  }
  ~ScopedWorkingDirectory()
  {
    fs::current_path(previous);
    fs::remove_all(path);
  }
  fs::path path;

private:
  fs::path previous;
};

} // namespace

// Several processes loading different models from the same directory must each parse their own model.
TEST(PromelaLoaderConcurrencyTest, ConcurrentLoadsInSameDirectoryDoNotMixModels)
{
  constexpr int processes = 8;
  constexpr int rounds = 20;

  std::vector<pid_t> children;
  for (int p = 0; p < processes; ++p) {
    auto name = "proc" + std::to_string(p);
    pid_t pid = fork();
    ASSERT_NE(pid, -1);
    if (pid == 0)
      _exit(loadRepeatedly(modelWithProctype(name), "proctype " + name + "(", rounds));
    children.push_back(pid);
  }

  int wrongModel = 0;
  int failed = 0;
  for (auto pid : children) {
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 2)
      ++wrongModel;
    else if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
      ++failed;
  }
  EXPECT_EQ(wrongModel, 0) << "processes that silently parsed another process's model";
  EXPECT_EQ(failed, 0) << "processes that crashed or exited with an error";
}

TEST(PromelaLoaderConcurrencyTest, LeavesNoWorkingFilesInCurrentDirectory)
{
  ScopedWorkingDirectory dir;
  {
    std::ofstream("model.pml") << modelWithProctype("p");
    promela_loader loader("model.pml");
  }

  std::vector<std::string> leftovers;
  for (const auto & entry : fs::directory_iterator(dir.path)) {
    auto name = entry.path().filename().string();
    if (name != "model.pml" && name != "fsm_graphvis")
      leftovers.push_back(name);
  }
  EXPECT_TRUE(leftovers.empty()) << "first leftover: " << (leftovers.empty() ? "" : leftovers.front());
}

// Models such as examples/models/adapro/*.pml use #include "./Theory.prp", resolved from the working directory.
TEST(PromelaLoaderConcurrencyTest, QuoteIncludesResolveFromCurrentDirectory)
{
  ScopedWorkingDirectory dir;
  std::ofstream("defs.prp") << "#define INIT 7\n";
  std::ofstream("model.pml") << "#include \"./defs.prp\"\n"
                                "byte x = INIT;\n"
                                "active proctype p(){\n"
                                "  x = 1;\n"
                                "}\n";

  promela_loader loader("model.pml");
  auto program = stmnt::string(loader.getProgram());
  EXPECT_NE(program.find("x = 7"), std::string::npos) << program;
}

// Transitions are identified by source line. The lexer takes every number on a cpp line marker as the line number,
// so the loader must not introduce file names containing digits into those markers.
TEST(PromelaLoaderConcurrencyTest, TransitionLinesFollowSourceLines)
{
  auto transitionLines = [](const std::string & file) {
    promela_loader loader(file);
    std::set<int> lines;
    for (auto edge : loader.getAutomata()->getTransitions())
      lines.insert(edge->getLineNb());
    return lines;
  };
  const std::string body = "byte x = 0;\n"
                           "active proctype p(){\n"
                           "  x = 1;\n"
                           "  x = 2\n"
                           "}\n";

  ScopedWorkingDirectory dir;
  std::ofstream("plain.pml") << body;
  std::ofstream("defs.prp") << "#define A 1\n"
                               "#define B 2\n";
  std::ofstream("included.pml") << "#include \"./defs.prp\"\n" << body;

  auto plain = transitionLines("plain.pml");
  ASSERT_FALSE(plain.empty());
  EXPECT_GE(*plain.begin(), 1);
  EXPECT_LE(*plain.rbegin(), 5);

  std::set<int> shiftedByInclude;
  for (int line : plain)
    shiftedByInclude.insert(line + 1);
  EXPECT_EQ(transitionLines("included.pml"), shiftedByInclude);
}

// Mutants that do not parse are common in a campaign; a failed load must not leave its scratch directory behind.
TEST(PromelaLoaderConcurrencyTest, FailedLoadRemovesScratchDirectory)
{
  ScopedWorkingDirectory tmp; // used as a private TMPDIR, so concurrent tests cannot interfere
  const char * previous = getenv("TMPDIR");
  std::string saved = previous ? previous : "";
  setenv("TMPDIR", tmp.path.c_str(), 1);

  EXPECT_EXIT(promela_loader("active proctype p(){ this is not promela }"), ::testing::ExitedWithCode(1), "Syntax error");
  EXPECT_TRUE(fs::is_empty(tmp.path));

  if (previous)
    setenv("TMPDIR", saved.c_str(), 1);
  else
    unsetenv("TMPDIR");
}

// A fork()ed child inherits the loader object; destroying that copy must not delete the parent's scratch directory.
TEST(PromelaLoaderConcurrencyTest, ForkedChildKeepsParentScratchDirectory)
{
  ScopedWorkingDirectory tmp; // used as a private TMPDIR, so concurrent tests cannot interfere
  ScopedWorkingDirectory cwd; // keeps fsm_graphvis out of that TMPDIR
  const char * previous = getenv("TMPDIR");
  std::string saved = previous ? previous : "";
  setenv("TMPDIR", tmp.path.c_str(), 1);

  std::optional<promela_loader> loader;
  loader.emplace(modelWithProctype("p"));
  pid_t pid = fork();
  ASSERT_NE(pid, -1);
  if (pid == 0) {
    loader.reset();
    _exit(0);
  }
  int status = 0;
  waitpid(pid, &status, 0);
  EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
  EXPECT_FALSE(fs::is_empty(tmp.path)) << "the child removed the parent's scratch directory";

  loader.reset();
  EXPECT_TRUE(fs::is_empty(tmp.path));

  if (previous)
    setenv("TMPDIR", saved.c_str(), 1);
  else
    unsetenv("TMPDIR");
}

// The model path reaches cpp through the shell; quotes in it must not break (or inject into) the command.
TEST(PromelaLoaderConcurrencyTest, ModelPathWithQuoteLoads)
{
  ScopedWorkingDirectory dir;
  std::ofstream("it's; touch injected.pml") << modelWithProctype("quoted");

  promela_loader loader("it's; touch injected.pml");
  EXPECT_NE(stmnt::string(loader.getProgram()).find("proctype quoted("), std::string::npos);
  EXPECT_FALSE(fs::exists("injected.pml"));
}
