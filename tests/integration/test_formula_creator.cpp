#include <daedalux/feature/tvl.hpp>
#include <daedalux/promela/parser/promela_loader.hpp>
#include <daedalux/promela/semantic/variable/state/initState.hpp>
#include <daedalux/algorithm/traceGenerator.hpp>
#include <daedalux/core/semantic.hpp>
#include <daedalux/formulas.hpp>


#include "../TestFilesUtils.hpp"

#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

// Define a fixture for the tests
class FormulaCreatorTest : public ::testing::Test {
protected:
  void SetUp() override
  {
    // Common setup code that will be called before each test
    std::string current_path = std::filesystem::current_path();
    testFilesUtils = std::make_unique<TestFilesUtils>(current_path);
  }

  void TearDown() override
  {
    // Common teardown code that will be called after each test
  }
  std::unique_ptr<TestFilesUtils> testFilesUtils;
};

TEST_F(FormulaCreatorTest, test_buildVariableValueMap_one_state)
{
  const TVL * tvl = nullptr;
  auto original_loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto fsm1 = original_loader->getAutomata();
  auto current_state_fsm1 = initState::createInitState(fsm1.get(), tvl);
  std::shared_ptr<state> current_state_fsm1_ptr(current_state_fsm1);
  const std::vector<std::shared_ptr<state>> states = std::vector<std::shared_ptr<state>>{current_state_fsm1_ptr};

  auto value_map = formulaCreator::buildVariableValueMap(states);
  for (auto var : value_map) {
    auto values = var.second;
    ASSERT_EQ(values.size(), 1);
  }
}

TEST_F(FormulaCreatorTest, test_buildVariableValueMap_two_states)
{
  const TVL * tvl = nullptr;
  auto original_loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto fsm1 = original_loader->getAutomata();
  auto current_state_fsm1 = initState::createInitState(fsm1.get(), tvl);
  std::shared_ptr<state> current_state_fsm1_ptr(current_state_fsm1);
  auto next_state = current_state_fsm1->Post().front()->Post().front()->Post().front()->Post().front()->Post().front();
  std::shared_ptr<state> next_state_ptr(next_state);
  const std::vector<std::shared_ptr<state>> states =
      std::vector<std::shared_ptr<state>>{current_state_fsm1_ptr, next_state_ptr};

  bool considerInternalVariables = false;
  ASSERT_EQ(states.size(), 2);
  ASSERT_FALSE(current_state_fsm1_ptr->isSame(next_state_ptr.get(), considerInternalVariables));

  auto value_map = formulaCreator::buildVariableValueMap(states);
  // Given that we have two states, the value map should at least for one variable have two values.
  bool has_two_values = false;
  for (auto var : value_map) {
    auto values = var.second;
    if (values.size() == 2) {
      has_two_values = true;
    }
    ASSERT_TRUE(values.size() <= 2);
  }
  ASSERT_TRUE(has_two_values);
}

TEST_F(FormulaCreatorTest, test_buildVariableValueMap_two_states_flows)
{
  const TVL * tvl = nullptr;
  auto original_loader = std::make_unique<promela_loader>(testFilesUtils->flows_model(), tvl);
  auto fsm1 = original_loader->getAutomata();
  auto current_state_fsm1 = initState::createInitState(fsm1.get(), tvl);
  std::shared_ptr<state> current_state_fsm1_ptr(current_state_fsm1);
  auto next_state = current_state_fsm1->Post().front()->Post().front()->Post().front();
  std::shared_ptr<state> next_state_ptr(next_state);
  const std::vector<std::shared_ptr<state>> states =
      std::vector<std::shared_ptr<state>>{current_state_fsm1_ptr, next_state_ptr};

  ASSERT_EQ(states.size(), 2);
  bool considerInternalVariables = false;
  ASSERT_FALSE(current_state_fsm1_ptr->isSame(next_state_ptr.get(), considerInternalVariables));

  auto value_map = formulaCreator::buildVariableValueMap(states);
  // Given that we have two states, the value map should at least for one variable have two values.
  bool has_two_values = false;
  for (auto var : value_map) {
    auto values = var.second;
    if (values.size() == 2) {
      has_two_values = true;
    }
    ASSERT_TRUE(values.size() <= 2);
  }
  ASSERT_TRUE(has_two_values);
}

TEST_F(FormulaCreatorTest, groupStates_singleState)
{
  const TVL * tvl = nullptr;
  auto file_path = testFilesUtils->flows_model();
  auto original_loader = std::make_unique<promela_loader>(file_path, tvl);
  auto fsm1 = original_loader->getAutomata();
  auto current_state_fsm1 = initState::createInitState(fsm1.get(), tvl);
  std::shared_ptr<state> current_state_fsm1_ptr(current_state_fsm1);
  const std::vector<std::shared_ptr<state>> states = std::vector<std::shared_ptr<state>>{current_state_fsm1_ptr};

  auto result = formulaCreator::groupStatesByFormula(states);
  auto var_a = std::make_shared<VariableFormula>("s.a");
  auto var_b = std::make_shared<VariableFormula>("s.b");
  auto var_c = std::make_shared<VariableFormula>("s.c");
  auto var_d = std::make_shared<VariableFormula>("s.d");
  auto formula_1 = std::make_shared<EqualsFormula>(var_a, std::make_shared<BooleanConstant>(false));
  auto formula_2 = std::make_shared<EqualsFormula>(var_b, std::make_shared<BooleanConstant>(false));
  auto formula_3 = std::make_shared<EqualsFormula>(var_c, std::make_shared<BooleanConstant>(false));
  auto formula_4 = std::make_shared<EqualsFormula>(var_d, std::make_shared<BooleanConstant>(false));

  std::vector<std::shared_ptr<formula>> formulas = {formula_1, formula_2, formula_3, formula_4};
  auto expected_result = formulaUtility::combineFormulas(formulas, CombinationOperatorType::AND_Symbol);
  ASSERT_TRUE(result->isEquivalent(*expected_result));
}

TEST_F(FormulaCreatorTest, groupStates_array)
{
  const TVL * tvl = nullptr;
  auto original_loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto fsm1 = original_loader->getAutomata();
  auto current_state_fsm1 = initState::createInitState(fsm1.get(), tvl);
  std::shared_ptr<state> current_state_fsm1_ptr(current_state_fsm1);
  bool considerInternalVariables = false;
  auto next_state = current_state_fsm1->Post().front()->Post().front()->Post().front()->Post().front()->Post().front();
  ASSERT_FALSE(current_state_fsm1_ptr->isSame(next_state, considerInternalVariables));
  auto next_next_state = next_state->Post().front()->Post().front()->Post().front()->Post().front()->Post().front();
  ASSERT_FALSE(next_state->isSame(next_next_state, considerInternalVariables));
  auto next_next_next_state = next_next_state->Post().front()->Post().front()->Post().front()->Post().front()->Post().front();
  ASSERT_FALSE(next_next_state->isSame(next_next_next_state, considerInternalVariables));
  std::shared_ptr<state> next_state_ptr(next_state);
  std::shared_ptr<state> next_next_state_ptr(next_next_state);
  std::shared_ptr<state> next_next_next_state_ptr(next_next_next_state);
  const std::vector<std::shared_ptr<state>> states = std::vector<std::shared_ptr<state>>{
      current_state_fsm1_ptr, next_state_ptr, next_next_state_ptr, next_next_next_state_ptr};

  ASSERT_EQ(states.size(), 4);
  auto result = formulaCreator::groupStatesByFormula(states);
  auto array_1 = std::make_shared<VariableFormula>("array[1]");
  auto array_2 = std::make_shared<VariableFormula>("array[2]");
  auto array_3 = std::make_shared<VariableFormula>("array[3]");
  auto i = std::make_shared<VariableFormula>("i");
  auto formula_1 =
      std::make_shared<AndFormula>(std::make_shared<LargerEqualsFormula>(array_1, std::make_shared<NumberConstant>(0)),
                                   std::make_shared<SmallerEqualsFormula>(array_1, std::make_shared<NumberConstant>(1)));
  auto formula_2 = std::make_shared<OrFormula>(std::make_shared<EqualsFormula>(array_2, std::make_shared<NumberConstant>(0)),
                                               std::make_shared<EqualsFormula>(array_2, std::make_shared<NumberConstant>(2)));
  auto formula_3 = std::make_shared<OrFormula>(std::make_shared<EqualsFormula>(array_3, std::make_shared<NumberConstant>(0)),
                                               std::make_shared<EqualsFormula>(array_3, std::make_shared<NumberConstant>(3)));

  auto equal_1 = std::make_shared<EqualsFormula>(i, std::make_shared<NumberConstant>(0));
  auto equal_2 = std::make_shared<EqualsFormula>(i, std::make_shared<NumberConstant>(1));
  auto equal_3 = std::make_shared<EqualsFormula>(i, std::make_shared<NumberConstant>(3));
  auto equal_4 = std::make_shared<EqualsFormula>(i, std::make_shared<NumberConstant>(4));
  std::vector<std::shared_ptr<formula>> equal_formulas = {equal_1, equal_2, equal_3, equal_4};
  auto formula4 = formulaUtility::combineFormulas(equal_formulas, CombinationOperatorType::OR_Symbol);

  auto formula_1_par = std::make_shared<ParenthesisFormula>(formula_1);
  auto formula_2_par = std::make_shared<ParenthesisFormula>(formula_2);
  auto formula_3_par = std::make_shared<ParenthesisFormula>(formula_3);
  auto formula4_par = std::make_shared<ParenthesisFormula>(formula4);
  std::vector<std::shared_ptr<formula>> formulas = {formula_1_par, formula_2_par, formula_3_par, formula4_par};
  auto expected_result = formulaUtility::combineFormulas(formulas, CombinationOperatorType::AND_Symbol);

  std::cout << "Result: " << result->toFormula() << std::endl;
  std::cout << "Expected: " << expected_result->toFormula() << std::endl;
  ASSERT_TRUE(result->isEquivalent(*expected_result));
}

TEST_F(FormulaCreatorTest, groupStates_flows)
{
  const TVL * tvl = nullptr;
  auto file_path = testFilesUtils->flows_model();
  auto original_loader = std::make_unique<promela_loader>(file_path, tvl);
  auto fsm1 = original_loader->getAutomata();
  auto current_state_fsm1 = initState::createInitState(fsm1.get(), tvl);
  std::shared_ptr<state> current_state_fsm1_ptr(current_state_fsm1);
  auto next_state = current_state_fsm1->Post().front();
  std::shared_ptr<state> next_state_ptr(next_state);
  const std::vector<std::shared_ptr<state>> states =
      std::vector<std::shared_ptr<state>>{current_state_fsm1_ptr, next_state_ptr};

  ASSERT_EQ(states.size(), 2);
  ASSERT_FALSE(current_state_fsm1_ptr->isSame(next_state_ptr.get(), false));
  bool temporal = true;
  auto result = formulaCreator::groupStatesByFormula(states, temporal);

  auto var_b = std::make_shared<VariableFormula>("b");
  auto var_c = std::make_shared<VariableFormula>("c");
  auto var_d = std::make_shared<VariableFormula>("d");
  auto formula_1 =
      std::make_shared<GloballyFormula>(std::make_shared<EqualsFormula>(var_b, std::make_shared<BooleanConstant>(false)));
  auto formula_2 =
      std::make_shared<GloballyFormula>(std::make_shared<EqualsFormula>(var_c, std::make_shared<BooleanConstant>(false)));
  auto formula_3 =
      std::make_shared<GloballyFormula>(std::make_shared<EqualsFormula>(var_d, std::make_shared<BooleanConstant>(false)));

  std::vector<std::shared_ptr<formula>> formulas = {formula_1, formula_2, formula_3};

  auto expected_result = formulaUtility::combineFormulas(formulas, CombinationOperatorType::AND_Symbol);
  std::cout << "Result: " << result->toFormula() << std::endl;
  std::cout << "Expected: " << expected_result->toFormula() << std::endl;
  // TODO look at this
  ASSERT_TRUE(result->isEquivalent(*expected_result));
}

TEST_F(FormulaCreatorTest, distinguishStates_array_same_states)
{
  const TVL * tvl = nullptr;
  auto file_path = testFilesUtils->array_model();
  auto original_loader = std::make_unique<promela_loader>(file_path, tvl);
  auto fsm1 = original_loader->getAutomata();
  auto current_state_fsm1 = initState::createInitState(fsm1.get(), tvl);
  std::shared_ptr<state> current_state_fsm1_ptr(current_state_fsm1);
  auto next_state = current_state_fsm1->Post().front()->Post().front()->Post().front()->Post().front()->Post().front();
  std::shared_ptr<state> next_state_ptr(next_state);
  const std::vector<std::shared_ptr<state>> include_states =
      std::vector<std::shared_ptr<state>>{current_state_fsm1_ptr, next_state_ptr};

  ASSERT_EQ(include_states.size(), 2);
  auto result = formulaCreator::distinguishStates(include_states, include_states);
  auto expected_result = std::make_shared<BooleanConstant>(false);
  std::cout << "Result: " << result->toFormula() << std::endl;
  std::cout << "Expected: " << expected_result->toFormula() << std::endl;
  // TODO look at this
  ASSERT_TRUE(result->isEquivalent(*expected_result));
}

TEST_F(FormulaCreatorTest, distinguishStates_array)
{
  const TVL * tvl = nullptr;
  auto file_path = testFilesUtils->array_model();
  auto original_loader = std::make_unique<promela_loader>(file_path, tvl);
  auto fsm1 = original_loader->getAutomata();
  auto current_state_fsm1 = initState::createInitState(fsm1.get(), tvl);
  std::shared_ptr<state> current_state_fsm1_ptr(current_state_fsm1);
  auto next_state = current_state_fsm1->Post().front()->Post().front()->Post().front()->Post().front()->Post().front();
  auto next_next_state = next_state->Post().front()->Post().front()->Post().front()->Post().front()->Post().front();
  auto next_next_next_state = next_next_state->Post().front()->Post().front()->Post().front()->Post().front()->Post().front();
  std::shared_ptr<state> next_state_ptr(next_state);
  std::shared_ptr<state> next_next_state_ptr(next_next_state);
  std::shared_ptr<state> next_next_next_state_ptr(next_next_next_state);
  const std::vector<std::shared_ptr<state>> include_states =
      std::vector<std::shared_ptr<state>>{current_state_fsm1_ptr, next_state_ptr};

  const std::vector<std::shared_ptr<state>> exclude_states =
      std::vector<std::shared_ptr<state>>{next_next_state_ptr, next_next_next_state_ptr};

  ASSERT_EQ(exclude_states.size(), 2);
  bool temporal = true;
  auto result = formulaCreator::distinguishStates(include_states, exclude_states, temporal);
  auto var_i = std::make_shared<VariableFormula>("i");
  auto array_1 = std::make_shared<VariableFormula>("array[1]");
  auto array_2 = std::make_shared<VariableFormula>("array[2]");
  auto array_3 = std::make_shared<VariableFormula>("array[3]");

  auto formula_1 = std::make_shared<EqualsFormula>(array_3, std::make_shared<NumberConstant>(0));
  auto formula_2 = std::make_shared<SmallerEqualsFormula>(array_2, std::make_shared<NumberConstant>(0));

  auto array_1_equal_0 = std::make_shared<EqualsFormula>(array_1, std::make_shared<NumberConstant>(0));
  auto array_1_equal_1 = std::make_shared<EqualsFormula>(array_1, std::make_shared<NumberConstant>(1));
  std::vector<std::shared_ptr<formula>> array_1_formulas = {array_1_equal_0, array_1_equal_1};
  auto array_1_formula =
      formulaUtility::combineFormulas(array_1_formulas, CombinationOperatorType::OR_Symbol);
  auto formula_3 = std::make_shared<ParenthesisFormula>(array_1_formula);
  auto var_i_lessThan_1 = std::make_shared<SmallerEqualsFormula>(var_i, std::make_shared<NumberConstant>(1));
  std::vector<std::shared_ptr<formula>> formulas = {formula_1, formula_2, formula_3, var_i_lessThan_1};

  auto form = formulaUtility::combineFormulas(formulas, CombinationOperatorType::AND_Symbol);
  auto expected_result = std::make_shared<GloballyFormula>(form);
  std::cout << "Result: " << result->toFormula() << std::endl;
  std::cout << "Expected: " << expected_result->toFormula() << std::endl;

  ASSERT_TRUE(result->isEquivalent(*expected_result));
}

TEST_F(FormulaCreatorTest, distinguishStates_flows)
{
  const TVL * tvl = nullptr;
  auto file_path = testFilesUtils->flows_model();
  auto original_loader = std::make_unique<promela_loader>(file_path, tvl);
  auto fsm1 = original_loader->getAutomata();
  auto current_state_fsm1 = initState::createInitState(fsm1.get(), tvl);
  std::shared_ptr<state> current_state_fsm1_ptr(current_state_fsm1);
  auto successors = current_state_fsm1->Post();
  auto next_state = successors.front();
  std::shared_ptr<state> next_state_ptr(next_state);
  const std::vector<std::shared_ptr<state>> include_states = std::vector<std::shared_ptr<state>>{
      current_state_fsm1_ptr,
  };
  const std::vector<std::shared_ptr<state>> exclude_states = std::vector<std::shared_ptr<state>>{next_state_ptr};
  ASSERT_EQ(include_states.size(), 1);
  ASSERT_EQ(exclude_states.size(), 1);
  auto include_state = include_states.front();
  auto exclude_state = exclude_states.front();
  ASSERT_FALSE(include_state->isSame(exclude_state.get(), true));
  bool temporal = true;
  auto result = formulaCreator::distinguishStates(include_states, exclude_states, temporal);
  auto var_a = std::make_shared<VariableFormula>("s.a");
  auto negated_a = std::make_shared<NotFormula>(var_a);
  auto expected_result = std::make_shared<GloballyFormula>(negated_a);
  std::cout << "Result: " << result->toFormula() << std::endl;
  std::cout << "Expected: " << expected_result->toFormula() << std::endl;
  ASSERT_TRUE(result->isEquivalent(*expected_result));
}

TEST_F(FormulaCreatorTest, ArrayFormulaFromTrace)
{
  const TVL * tvl = nullptr;
  auto file_path = testFilesUtils->array_model_original();
  LTLClaimsProcessor::removeClaimFromFile(file_path);
  auto original_loader = new promela_loader(file_path, tvl);
  auto originalFSM = original_loader->getAutomata();
  delete original_loader;
  auto file_path_mutant = testFilesUtils->array_model_mutant();
  LTLClaimsProcessor::removeClaimFromFile(file_path_mutant);
  auto mutant_loader = std::make_unique<promela_loader>(file_path_mutant, tvl);
  auto mutantFSM = mutant_loader->getAutomata();
  auto trace_size = 12;
  auto traceGen = std::make_unique<TraceGenerator>(originalFSM, mutantFSM);
  auto traceReport = traceGen->generateTraceReport(1, trace_size);
  auto good_trace = *traceReport->getGoodTraces().begin();
  auto bad_trace = *traceReport->getBadTraces().begin();
  auto result = formulaCreator::distinguishTraces(good_trace, bad_trace);
  auto array_3 = std::make_shared<VariableFormula>("array[3]");
  auto form = std::make_shared<LargerEqualsFormula>(array_3, std::make_shared<NumberConstant>(3));
  auto eventualFormula = std::make_shared<FinallyFormula>(form);
  auto nextFormula = std::make_shared<NextFormula>(eventualFormula);
  std::cout << "Result: " << result->toFormula() << std::endl;
  // TODO: fix this
  //  std::cout << "Expected: " << expected_result->toFormula() << std::endl;
  //  ASSERT_TRUE(result->isEquivalent(expected_result));
}

TEST_F(FormulaCreatorTest, formulaStringToNeverClaim_Globally)
{
  auto formula = "[](x)";
  auto result = LTLClaimsProcessor::transformLTLStringToNeverClaim(formula);
  std::string expected_result =
      "never{/*!([](x))*/\nT0_init:\n\tif\n\t::(1)->gotoT0_init\n\t::(!x)->gotoaccept_all\n\tfi;\naccept_all:\n\tskip\n}\n";
  expected_result.erase(remove(expected_result.begin(), expected_result.end(), ' '), expected_result.end());
  result.erase(remove(result.begin(), result.end(), ' '), result.end());
  ASSERT_EQ(result, expected_result);
}

TEST_F(FormulaCreatorTest, formulaStringToNeverClaim_Finally)
{
  auto formula = "<>(x)";
  auto result = LTLClaimsProcessor::transformLTLStringToNeverClaim(formula);
  std::string expected_result = "never{/*!(<>(x))*/\naccept_init:\n\tif\n\t::(!x)->gotoaccept_init\n\tfi;\n}\n";
  expected_result.erase(remove(expected_result.begin(), expected_result.end(), ' '), expected_result.end());
  result.erase(remove(result.begin(), result.end(), ' '), result.end());
  ASSERT_EQ(result, expected_result);
}

TEST_F(FormulaCreatorTest, formulaStringToNeverClaim_Liveness)
{
  auto formula = "[]((!(x)) -> <>x)";
  auto result = LTLClaimsProcessor::transformLTLStringToNeverClaim(formula);
  std::string expected_result = "never{/*!([]((!(x))-><>x))*/"
                                "\nT0_init:\n\tif\n\t::(1)->gotoT0_init\n\t::(!x)->gotoaccept_S2\n\tfi;\naccept_S2:\n\tif\n\t::"
                                "(!x)->gotoaccept_S2\n\tfi;\n}\n";
  expected_result.erase(remove(expected_result.begin(), expected_result.end(), ' '), expected_result.end());
  result.erase(remove(result.begin(), result.end(), ' '), result.end());
  ASSERT_EQ(result, expected_result);
}

// TEST_F(FormulaCreatorTest, formulaFromTrace)
// {
//   auto result = LTLClaimsProcessor::transformLTLStringToNeverClaim(formula);
//   std::cout << "Result: " << result << std::endl;
//   std::string expected_result = "never{/*!(<>(x))*/\naccept_init:\n\tif\n\t::(!x)->gotoaccept_init\n\tfi;\n}\n";
//   expected_result.erase(remove(expected_result.begin(), expected_result.end(), ' '), expected_result.end());
//   result.erase(remove(result.begin(), result.end(), ' '), result.end());
//   ASSERT_EQ(result, expected_result);
// }

// TEST_F(FormulaCreatorTest, formulaFromTrace)
// {
//   auto result = LTLClaimsProcessor::transformLTLStringToNeverClaim(formula);
//   expected_result.erase(remove(expected_result.begin(), expected_result.end(), ' '), expected_result.end());
//   result.erase(remove(result.begin(), result.end(), ' '), result.end());
//   ASSERT_EQ(result, expected_result);
// }
