#include <daedalux/mutants/mutantAnalyzer.hpp>

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace {

// Two assignments, so there are at least two mutation points.
const std::string model = "byte x = 0;\n"
                          "active proctype p(){\n"
                          "  x = 1;\n"
                          "  x = x + 2\n"
                          "}\n";

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

// A model given without a directory used to be taken as the folder itself: "model.pml/mutants" (#20).
TEST(MutantOutputFolderTest, BareFileNameWritesMutantsNextToTheModel)
{
  ScopedWorkingDirectory dir;
  std::ofstream("model.pml") << model;

  const std::string path = "model.pml"; // named: MutantAnalyzer keeps a reference to it (#24)
  MutantAnalyzer analyzer(path);
  EXPECT_NO_THROW(analyzer.createMutants(1));
  ASSERT_EQ(analyzer.getMutantFilePaths().size(), 1u);
  EXPECT_EQ(analyzer.getMutantFilePaths().front(), "mutants/mutant_1.pml");
  EXPECT_TRUE(fs::exists("mutants/mutant_1.pml"));
  EXPECT_TRUE(fs::exists("mutants/original.pml"));
}

// Paths with a directory keep writing to <dir>/mutants, as before.
TEST(MutantOutputFolderTest, PathWithDirectoryWritesMutantsInThatDirectory)
{
  ScopedWorkingDirectory dir;
  fs::create_directory("models");
  std::ofstream("models/model.pml") << model;

  const std::string path = "models/model.pml";
  MutantAnalyzer analyzer(path);
  analyzer.createMutants(1);
  ASSERT_EQ(analyzer.getMutantFilePaths().size(), 1u);
  EXPECT_EQ(analyzer.getMutantFilePaths().front(), "models/mutants/mutant_1.pml");
  EXPECT_TRUE(fs::exists("models/mutants/original.pml"));
}
