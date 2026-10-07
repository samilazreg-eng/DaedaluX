#pragma once

#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <system_error>

// A writable copy for one test case. The configured fixture tree is input only.
// Changing cwd also contains incidental relative outputs from product code.
// Use only in tests whose threads/children finish before the workspace is destroyed.
class TestWorkspace {
public:
  TestWorkspace() : previous(std::filesystem::current_path())
  {
    auto pattern = (std::filesystem::temp_directory_path() / "daedalux-fixtures-XXXXXX").string();
    if (mkdtemp(pattern.data()) == nullptr)
      throw std::runtime_error("Could not create private test workspace");
    directory = pattern;
    try {
      const auto inputs = std::filesystem::canonical(std::filesystem::path(DAEDALUX_TEST_FIXTURES_DIR) / "test_files");
      std::filesystem::copy(inputs, directory / "test_files",
                            std::filesystem::copy_options::recursive);
      std::filesystem::current_path(directory);
    }
    catch (...) {
      std::error_code error;
      std::filesystem::remove_all(directory, error);
      throw;
    }
  }

  ~TestWorkspace()
  {
    std::error_code error;
    std::filesystem::current_path(previous, error);
    // Never remove the directory if it is still the process working directory.
    if (!error)
      std::filesystem::remove_all(directory, error);
  }

  TestWorkspace(const TestWorkspace &) = delete;
  TestWorkspace & operator=(const TestWorkspace &) = delete;

  const std::filesystem::path & path() const { return directory; }

private:
  std::filesystem::path previous;
  std::filesystem::path directory;
};
