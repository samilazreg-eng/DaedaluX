#include "TestEnvironment.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>

#include <unistd.h>

namespace fs = std::filesystem;

namespace {

class TestWorkspace {
public:
  // The directory is named after the case: a case that crashes leaves it behind.
  explicit TestWorkspace(std::string name) : creator(getpid()), previous(fs::current_path())
  {
    std::replace(name.begin(), name.end(), '/', '_'); // the names of parameterized cases contain one
    fs::create_directories(DAEDALUX_TEST_RUNTIME_DIR);
    auto pattern = (fs::path(DAEDALUX_TEST_RUNTIME_DIR) / (name + ".XXXXXX")).string();
    if (mkdtemp(pattern.data()) == nullptr)
      throw std::runtime_error("Could not create the test workspace " + pattern);
    directory = pattern;
    fs::current_path(directory);
  }

  // A fork()ed child inherits the workspace and reaches this when it leaves through exit(): only the creator removes it.
  ~TestWorkspace()
  {
    if (creator != getpid())
      return;
    std::error_code ignored;
    fs::current_path(previous, ignored);
    fs::remove_all(directory, ignored);
  }

  TestWorkspace(const TestWorkspace &) = delete;
  TestWorkspace & operator=(const TestWorkspace &) = delete;

  const fs::path & path() const { return directory; }

private:
  pid_t creator;
  fs::path previous;
  fs::path directory;
};

std::optional<TestWorkspace> workspace;

class WorkspacePerTest : public ::testing::EmptyTestEventListener {
  void OnTestStart(const ::testing::TestInfo & test) override
  {
    workspace.emplace(std::string(test.test_suite_name()) + "." + test.name());
  }
  void OnTestEnd(const ::testing::TestInfo &) override { workspace.reset(); }
};

} // namespace

void installTestEnvironment()
{
  ::testing::UnitTest::GetInstance()->listeners().Append(new WorkspacePerTest); // GoogleTest owns the listener
}

const fs::path & testWorkspace() { return workspace.value().path(); }
