#include <filesystem>
#include <daedalux/feature/tvl.hpp>
#include <daedalux/algorithm/ltlModelChecker.hpp>
#include <daedalux/core/logic.hpp>
#include <gtest/gtest.h>
#include <memory>

class LtlModelCheckerTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    // Common setup code that will be called before each test
    modelChecker = std::make_unique<ltlModelChecker>();
  }
  void TearDown() override
  {
    // Common teardown code that will be called after each test
  }

  bool isSatisfied(const std::string & filePath)
  {
    const TVL * tvl = nullptr;
    std::string currentPath = std::filesystem::current_path();
    auto file_path = currentPath + filePath;
    return modelChecker->check(file_path);
  }

  std::unique_ptr<ltlModelChecker> modelChecker;
  std::string promela_ltl_model = "/test_files/ltl/ltl.pml";
  std::string promela_ltl_model1 = "/test_files/ltl/liveness_1.pml";
  std::string promela_ltl_model2 = "/test_files/ltl/liveness_2.pml";
  std::string promela_ltl_model3 = "/test_files/ltl/liveness_3.pml";
  std::string promela_ltl_model4 = "/test_files/ltl/liveness_4.pml";
  std::string promela_multiLTL = "/test_files/ltl/ltl_multi.pml";
};

TEST_F(LtlModelCheckerTest, ltlModelShouldBeSatisfied) { ASSERT_TRUE(isSatisfied(promela_ltl_model)); }

TEST_F(LtlModelCheckerTest, liveness1ShouldBeSatisfied) { ASSERT_TRUE(isSatisfied(promela_ltl_model1)); }

// ASK Sami about all the tests below
// TEST_F(LtlModelCheckerTest, liveness2ShouldBeSatisfied) { 
//   ASSERT_TRUE(isSatisfied(promela_ltl_model2)); 
// }

// TEST_F(LtlModelCheckerTest, liveness3ShouldBeSatisfied) { ASSERT_TRUE(isSatisfied(promela_ltl_model3)); }

// TEST_F(LtlModelCheckerTest, liveness4ShouldBeSatisfied) { ASSERT_TRUE(isSatisfied(promela_ltl_model4)); }

// TEST_F(LtlModelCheckerTest, multiLtlModelShouldBeSatisfied) { ASSERT_TRUE(isSatisfied(promela_multiLTL)); }
