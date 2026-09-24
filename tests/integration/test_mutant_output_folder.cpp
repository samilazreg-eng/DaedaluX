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
  EXPECT_EQ(analyzer.getMutantFilePaths().front(), "model_mutants/mutant_1.pml");
  EXPECT_TRUE(fs::exists("model_mutants/mutant_1.pml"));
  EXPECT_TRUE(fs::exists("model_mutants/original.pml"));
}

// Paths with a directory write to <dir>/<model>_mutants.
TEST(MutantOutputFolderTest, PathWithDirectoryWritesMutantsInThatDirectory)
{
  ScopedWorkingDirectory dir;
  fs::create_directory("models");
  std::ofstream("models/model.pml") << model;

  const std::string path = "models/model.pml";
  MutantAnalyzer analyzer(path);
  analyzer.createMutants(1);
  ASSERT_EQ(analyzer.getMutantFilePaths().size(), 1u);
  EXPECT_EQ(analyzer.getMutantFilePaths().front(), "models/model_mutants/mutant_1.pml");
  EXPECT_TRUE(fs::exists("models/model_mutants/original.pml"));
}

// Models in one directory used to share <dir>/mutants, which each run removed first (#19).
TEST(MutantOutputFolderTest, ModelsInOneDirectoryKeepTheirOwnMutants)
{
  ScopedWorkingDirectory dir;
  std::ofstream("first.pml") << "byte x = 0;\nactive proctype first(){\n  x = 1;\n  x = x + 2\n}\n";
  std::ofstream("second.pml") << "byte x = 0;\nactive proctype second(){\n  x = 1;\n  x = x + 2\n}\n";

  const std::string firstPath = "first.pml";
  const std::string secondPath = "second.pml";
  MutantAnalyzer first(firstPath);
  first.createMutants(2);
  MutantAnalyzer second(secondPath);
  second.createMutants(2);

  ASSERT_EQ(first.getMutantFilePaths().size(), 2u);
  for (const auto & mutant : first.getMutantFilePaths()) {
    std::ifstream in(mutant);
    std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_NE(text.find("proctype first("), std::string::npos) << mutant << " was replaced or deleted by the second run";
  }
}

// A mutants/ folder the run did not create, such as a committed mutant dataset, must survive (#19).
TEST(MutantOutputFolderTest, ExistingMutantsFolderIsLeftAlone)
{
  ScopedWorkingDirectory dir;
  fs::create_directory("mutants");
  std::ofstream("mutants/dataset.pml") << model;
  std::ofstream("model.pml") << model;

  const std::string path = "model.pml";
  MutantAnalyzer analyzer(path);
  analyzer.createMutants(1);
  EXPECT_TRUE(fs::exists("mutants/dataset.pml"));
}
