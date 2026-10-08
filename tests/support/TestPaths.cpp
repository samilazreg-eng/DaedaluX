#include "TestPaths.hpp"

#include "TestEnvironment.hpp"

#include <filesystem>

namespace fs = std::filesystem;

std::string sharedTestFile(const std::string & name) { return (fs::path(DAEDALUX_TEST_DATA_DIR) / name).string(); }

std::string privateTestFile(const std::string & name)
{
  auto copy = testWorkspace() / "data" / name;
  if (!fs::exists(copy)) {
    fs::create_directories(copy.parent_path());
    fs::copy_file(sharedTestFile(name), copy);
  }
  return copy.string();
}
