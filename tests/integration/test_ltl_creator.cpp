#include <daedalux/core/logic/ltl.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>

// Define a fixture for the tests
class LTLTransformerTest : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override
  {
    // Common teardown code that will be called after each test
  }
  std::string flows_model = "/test_files/appendClaimTest/flows.pml";
  std::string current_path = std::filesystem::current_path();

  bool appendClaimTest(const std::string & filePath, const std::string & expected_file, const std::string formula)
  {
    auto tempFilePath = current_path + "/test_files/appendClaimTest/flows_temp.pml";
    std::filesystem::copy_file(filePath, tempFilePath, std::filesystem::copy_options::overwrite_existing);
    LTLClaimsProcessor::appendClaimToFile(tempFilePath, formula);
    // Compare the temporary file with the expected file
    auto compareResult = compareFiles(tempFilePath, expected_file);
    // Remove the temporary file
    std::filesystem::remove(tempFilePath);
    return compareResult;
  }

  void removeWhitespace(std::string & str)
  {
    // Remove characters if they match the predicate
    str.erase(std::remove_if(str.begin(), str.end(), isspace), str.end());
  }

  bool compareLines(std::string & line1, std::string & line2) 
  {
    // This function compares two strings after removing all spaces
    removeWhitespace(line1);
    removeWhitespace(line2);
    return line1.compare(line2) == 0;
  }

  bool compareFiles(const std::string & filePath1, const std::string & filePath2)
  {
    std::ifstream file1(filePath1), file2(filePath2);
    std::string line1, line2;

    if (!file1.is_open() || !file2.is_open()) {
      std::cerr << "Error opening one of the files." << std::endl;
      return false;
    }
    int line_number = 0;
    while (!file1.eof() || !file2.eof()) {
      std::getline(file1, line1);
      std::getline(file2, line2);
      auto equal = compareLines(line1, line2);
      auto file1_eof = file1.eof();
      auto file2_eof = file2.eof();
      auto equal_eof = file1_eof == file2_eof;
      if (!equal_eof || !equal) {
        std::cout << "Files are different at line " << line_number << std::endl;
        std::cout << filePath1 << ": " << line1 << std::endl;
        std::cout << filePath2 << ": " << line2 << std::endl;
        return false; // Files are different
      }
      line_number++;
    }
    return true; // Files are identical
  }
};

TEST_F(LTLTransformerTest, formulaStringToNeverClaim_Globally)
{
  auto formula = "[](x)";
  auto result = LTLClaimsProcessor::transformLTLStringToNeverClaim(formula);
  std::string expected_result =
      "never{/*!([](x))*/\nT0_init:\n\tif\n\t::(1)->gotoT0_init\n\t::(!x)->gotoaccept_all\n\tfi;\naccept_all:\n\tskip\n}\n";
  ASSERT_TRUE(compareLines(result, expected_result));
}

TEST_F(LTLTransformerTest, formulaStringToNeverClaim_Finally)
{
  auto formula = "<>(x)";
  auto result = LTLClaimsProcessor::transformLTLStringToNeverClaim(formula);
  std::string expected_result = "never{/*!(<>(x))*/\naccept_init:\n\tif\n\t::(!x)->gotoaccept_init\n\tfi;\n}\n";
  ASSERT_TRUE(compareLines(result, expected_result));
}

TEST_F(LTLTransformerTest, formulaStringToNeverClaim_Liveness)
{
  auto formula = "[]((!(x)) -> <>x)";
  auto result = LTLClaimsProcessor::transformLTLStringToNeverClaim(formula);
  std::string expected_result = "never{/*!([]((!(x))-><>x))*/"
                                "\nT0_init:\n\tif\n\t::(1)->gotoT0_init\n\t::(!x)->gotoaccept_S2\n\tfi;\naccept_S2:\n\tif\n\t::"
                                "(!x)->gotoaccept_S2\n\tfi;\n}\n";
  ASSERT_TRUE(compareLines(result, expected_result));
}

TEST_F(LTLTransformerTest, AppendNeverClaim_Globally)
{
  auto filePath = current_path + flows_model;
  std::string expected_file = current_path + "/test_files/appendClaimTest/flows_always_expected.pml";
  auto formula = "[](x)";
  auto result = appendClaimTest(filePath, expected_file, formula);
  ASSERT_TRUE(result);
}

TEST_F(LTLTransformerTest, AppendToNeverClaim_Finally)
{
  auto filePath = current_path + flows_model;
  auto formula = "<>(x)";
  std::string expected_file = current_path + "/test_files/appendClaimTest/flows_eventually_expected.pml";
  auto result = appendClaimTest(filePath, expected_file, formula);
  ASSERT_TRUE(result);
}

TEST_F(LTLTransformerTest, AppendToNeverClaim_Liveness)
{
  auto filePath = current_path + flows_model;
  std::string expected_file = current_path + "/test_files/appendClaimTest/flows_liveness_expected.pml";
  auto formula = "[]((!(x)) -> <>x)";
  auto result = appendClaimTest(filePath, expected_file, formula);
  ASSERT_TRUE(result);
}