#include <daedalux/mutants/mutantAnalyzer.hpp>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

#include <daedalux/algorithm/fsmExplorer.hpp>
#include <daedalux/algorithm/traceGenerator.hpp>
#include <daedalux/formulas/formulaCreator.hpp>
#include <daedalux/formulas/predicates/binaryPredicate.hpp>
#include <daedalux/formulas/predicates/statePredicate.hpp>
#include <daedalux/formulas/predicates/valuesPredicate.hpp>

#include "../TestFilesUtils.hpp"

// Define a fixture for the tests
class TraceGeneratorTest : public ::testing::Test {
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

TEST_F(TraceGeneratorTest, SimpleTraceHelloWorld)
{
  const TVL * tvl = nullptr;
  auto original_loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto fsm = original_loader->getAutomata();
  auto trace_size = 15;
  auto trace = TraceGenerator::generateTrace(fsm, trace_size);
  ASSERT_EQ(trace->size(), trace_size);
  // Let's print the trace to the console
  trace->printCSV(std::cout);
}

TEST_F(TraceGeneratorTest, ProjectSimpleTraceArray)
{
  const TVL * tvl = nullptr;
  auto original_loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto fsm = original_loader->getAutomata();
  auto trace_size = 15;
  auto trace = TraceGenerator::generateTrace(fsm, trace_size);
  auto constant_1 = std::make_shared<ConstantPredicate<int>>(1);
  auto constant_3 = std::make_shared<ConstantPredicate<int>>(3);
  auto array_1 = std::make_shared<VariablePredicate<int>>("array[1]");
  auto array_3 = std::make_shared<VariablePredicate<int>>("array[3]");
  std::string pred_name_1 = "constant_1_eq_1";
  std::string pred_name_2 = "array_1_eq_1";

  std::string pred_name_3 = "array_3_lessThan_3";
  auto constant_1_eq_1 = std::make_shared<EqualityPredicate<int>>(pred_name_1, constant_1, constant_1);
  auto array_1_eq_1 = std::make_shared<EqualityPredicate<int>>(pred_name_2, array_1, constant_1);
  auto array_3_lessThan_3 = std::make_shared<LessThanPredicate<int>>(pred_name_3, array_3, constant_3);

  std::vector<std::shared_ptr<statePredicate>> predicates = {constant_1_eq_1, array_1_eq_1, array_3_lessThan_3};
  auto result = trace->projectTrace(predicates);
  auto expected = "1, 0, 1; 1, 0, 1; 1, 0, 1; 1, 0, 1; 1, 0, 1; 1, 1, 1; 1, 1, 1; 1, 1, 1; 1, 1, 1; 1, 1, 1; 1, 1, 1; 1, 1, 0; 1, 1, 0; 1, 1, 0; 1, 1, 0";
  ASSERT_EQ(result, expected);
}

TEST_F(TraceGeneratorTest, ProjectSimpleTraceTrafficLight)
{
  const TVL * tvl = nullptr;
  auto original_loader = std::make_unique<promela_loader>(testFilesUtils->trafficLight_model_original(), tvl);
  auto fsm = original_loader->getAutomata();
  auto trace_size = 15;
  auto trace = TraceGenerator::generateTrace(fsm, trace_size);
  auto constant_yellow = std::make_shared<ConstantPredicate<std::string>>("yellow");
  auto constant_red = std::make_shared<ConstantPredicate<std::string>>("red");
  auto constant_green = std::make_shared<ConstantPredicate<std::string>>("green");
  auto state = std::make_shared<VariablePredicate<std::string>>("state");
  std::string pred_name_1 = "red_light";
  std::string pred_name_2 = "yellow_light";
  std::string pred_name_3 = "green_light";
  auto red_light = std::make_shared<EqualityPredicate<std::string>>(pred_name_1, state, constant_red);
  auto yellow_light = std::make_shared<EqualityPredicate<std::string>>(pred_name_2, state, constant_yellow);
  auto green_light = std::make_shared<EqualityPredicate<std::string>>(pred_name_3, state, constant_green);

  std::vector<std::shared_ptr<statePredicate>> predicates = {red_light, yellow_light, green_light};
  auto result = trace->projectTrace(predicates);
  auto expected = "1, 0, 0; 1, 0, 0; 0, 0, 1; 0, 0, 1; 0, 1, 0; 0, 1, 0; 1, 0, 0; 1, 0, 0; 0, 0, 1; 0, 0, 1; 0, 1, 0; 0, 1, 0; 1, 0, 0; 1, 0, 0; 0, 0, 1";
  ASSERT_EQ(result, expected);
}


TEST_F(TraceGeneratorTest, DiscriminatingTrace_SameFSM_NonDifferentiatingTrace)
{
  const TVL * tvl = nullptr;
  auto original_loader = std::make_unique<promela_loader>(testFilesUtils->array_model(), tvl);
  auto originalFSM = original_loader->getAutomata();
  auto trace_size = 15;
  auto traceGen = std::make_unique<TraceGenerator>(originalFSM, originalFSM);
  auto trace = traceGen->generateNegativeTrace(trace_size);
  ASSERT_FALSE(trace->isDifferentiatingTrace());
  ASSERT_EQ(trace->size(), trace_size);
}

TEST_F(TraceGeneratorTest, DiscriminatingTrace_DifferentFSMs_DifferentiatingTrace)
{
  const TVL * tvl = nullptr;
  auto original_loader = new promela_loader(testFilesUtils->array_model(), tvl);
  auto originalFSM = original_loader->getAutomata();
  delete original_loader;
  auto file_path_mutant = testFilesUtils->array_model_mutant();
  LTLClaimsProcessor::removeClaimFromFile(file_path_mutant);
  auto mutant_loader = std::make_unique<promela_loader>(file_path_mutant, tvl);
  auto mutantFSM = mutant_loader->getAutomata();
  auto trace_size = 15;
  auto traceGen = std::make_unique<TraceGenerator>(originalFSM, mutantFSM);
  // We should be able to create a trace that only the original FSM can satisfy
  auto trace = traceGen->generatePositiveTrace(trace_size);
  ASSERT_TRUE(trace->isDifferentiatingTrace());
  ASSERT_EQ(trace->size(), trace_size);
}

// TEST_F(TraceGeneratorTest, SimpleTraceHelloWorld_DifferentFSM_IgnoreCommonPrefix)
// {
//   const TVL * tvl = nullptr;
//   auto file_path = testFilesUtils->array_model();
//   LTLClaimsProcessor::removeClaimFromFile(file_path);
//   auto original_loader = new promela_loader(file_path, tvl);
//   auto originalFSM = original_loader->getAutomata();
//   delete original_loader;
//   auto file_path_mutant = testFilesUtils->array_model_mutant();
//   LTLClaimsProcessor::removeClaimFromFile(file_path_mutant);
//   auto mutant_loader = std::make_unique<promela_loader>(file_path_mutant, tvl);
//   auto mutantFSM = mutant_loader->getAutomata();
//   auto trace_size = 15;
//   bool ignore_common_prefix = true;
//   auto traceGen = std::make_unique<TraceGenerator>(originalFSM, mutantFSM);
//   auto trace = traceGen->generatePositiveTrace(trace_size, ignore_common_prefix);
//   // The trace will be shorter than the requested size as the common prefix is ignored
//   auto expected_trace_size = trace_size + 1;
//   ASSERT_EQ(trace->size(), expected_trace_size);
// }

TEST_F(TraceGeneratorTest, TraceReport_DifferentFSM)
{
  const TVL * tvl = nullptr;
  auto file_path = testFilesUtils->array_model();
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
  ASSERT_EQ(traceReport->getGoodTraces().size(), 1);
  ASSERT_EQ(traceReport->getBadTraces().size(), 1);
  auto good_trace = *traceReport->getGoodTraces().begin();
  auto bad_trace = *traceReport->getBadTraces().begin();
  ASSERT_EQ(good_trace->size(), trace_size);
  ASSERT_EQ(bad_trace->size(), trace_size);
}

TEST_F(TraceGeneratorTest, LogWithTraceReportToScarletArray)
{
  const TVL * tvl = nullptr;
  auto file_path = testFilesUtils->array_model();
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
  auto traceReport = traceGen->generateTraceReport(10, trace_size);

  auto constant_1 = std::make_shared<ConstantPredicate<int>>(1);
  auto constant_3 = std::make_shared<ConstantPredicate<int>>(1);
  auto array_1 = std::make_shared<VariablePredicate<int>>("array[1]");
  auto array_3 = std::make_shared<VariablePredicate<int>>("array[3]");
  std::string pred_name_2 = "array_1_eq_1";
  std::string pred_name_3 = "array_3_lessThan_3";
  auto array_1_eq_1 = std::make_shared<EqualityPredicate<int>>(pred_name_2, array_1, constant_1);
  auto array_3_lessThan_3 = std::make_shared<LessThanPredicate<int>>(pred_name_3, array_3, constant_3);

  std::vector<std::shared_ptr<statePredicate>> predicates = {array_1_eq_1, array_3_lessThan_3};

  auto trace_file = "trace_report_array.trace";
  std::ofstream file(trace_file);
  traceReport->printPredicatesEvaluation(file, predicates);
  file.close();
}

TEST_F(TraceGeneratorTest, LogWithTraceReportToScarletTrafficLight)
{
  const TVL * tvl = nullptr;
  auto file_path = testFilesUtils->trafficLight_model_original();
  LTLClaimsProcessor::removeClaimFromFile(file_path);
  auto original_loader = new promela_loader(file_path, tvl);
  auto originalFSM = original_loader->getAutomata();
  delete original_loader;
  auto file_path_mutant = testFilesUtils->trafficLight_model_mutant_alt();
  LTLClaimsProcessor::removeClaimFromFile(file_path_mutant);
  auto mutant_loader = std::make_unique<promela_loader>(file_path_mutant, tvl);
  auto mutantFSM = mutant_loader->getAutomata();
  auto trace_size = 12;
  auto traceGen = std::make_unique<TraceGenerator>(originalFSM, mutantFSM);
  auto traceReport = traceGen->generateTraceReport(10, trace_size);
  auto constant_yellow = std::make_shared<ConstantPredicate<std::string>>("yellow");
  auto constant_red = std::make_shared<ConstantPredicate<std::string>>("red");
  auto constant_green = std::make_shared<ConstantPredicate<std::string>>("green");
  auto state = std::make_shared<VariablePredicate<std::string>>("state");
  std::string pred_name_1 = "red_light";
  std::string pred_name_2 = "yellow_light";
  std::string pred_name_3 = "green_light";
  auto red_light = std::make_shared<EqualityPredicate<std::string>>(pred_name_1, state, constant_red);
  auto yellow_light = std::make_shared<EqualityPredicate<std::string>>(pred_name_2, state, constant_yellow);
  auto green_light = std::make_shared<EqualityPredicate<std::string>>(pred_name_3, state, constant_green);

  std::vector<std::shared_ptr<statePredicate>> predicates = {red_light, yellow_light, green_light};
  
  auto trace_file = "trace_report_trafficLight.trace";
  std::ofstream file(trace_file);
  traceReport->printPredicatesEvaluation(file, predicates);
  file.close();
}

TEST_F(TraceGeneratorTest, LogTraceReportToScarletTrafficLight)
{
  auto file_path = testFilesUtils->trafficLight_model_original();
  auto model_analyzer = std::make_unique<MutantAnalyzer>(file_path);
  auto constant_yellow = std::make_shared<ConstantPredicate<std::string>>("yellow");
  auto constant_red = std::make_shared<ConstantPredicate<std::string>>("red");
  auto constant_green = std::make_shared<ConstantPredicate<std::string>>("green");
  auto state = std::make_shared<VariablePredicate<std::string>>("state");
  std::string pred_name_1 = "red_light";
  std::string pred_name_2 = "yellow_light";
  std::string pred_name_3 = "green_light";
  auto red_light = std::make_shared<EqualityPredicate<std::string>>(pred_name_1, state, constant_red);
  auto yellow_light = std::make_shared<EqualityPredicate<std::string>>(pred_name_2, state, constant_yellow);
  auto green_light = std::make_shared<EqualityPredicate<std::string>>(pred_name_3, state, constant_green);

  std::vector<std::shared_ptr<statePredicate>> predicates = {red_light, yellow_light, green_light};
  
  std::string trace_file = "trace_report_trafficLight_scarlet.trace";
  model_analyzer->generateScarletFile(trace_file, 10, predicates);
}

TEST_F(TraceGeneratorTest, LogTraceReportToScarletTwoTrafficLights)
{
  auto file_path = testFilesUtils->two_trafficLight_model_original();
  auto model_analyzer = std::make_unique<MutantAnalyzer>(file_path);
  auto constant_yellow = std::make_shared<ConstantPredicate<std::string>>("yellow");
  auto constant_red = std::make_shared<ConstantPredicate<std::string>>("red");
  auto constant_green = std::make_shared<ConstantPredicate<std::string>>("green");
  auto state1 = std::make_shared<VariablePredicate<std::string>>("state1");
  auto state2 = std::make_shared<VariablePredicate<std::string>>("state2");
  std::string pred_name_1 = "state1_red_light";
  std::string pred_name_2 = "state1_yellow_light";
  std::string pred_name_3 = "state_1green_light";
  std::string pred_name_4 = "state2_red_light";
  std::string pred_name_5 = "state2_yellow_light";
  std::string pred_name_6 = "state2_1green_light";
  auto red_light = std::make_shared<EqualityPredicate<std::string>>(pred_name_1, state1, constant_red);
  auto yellow_light = std::make_shared<EqualityPredicate<std::string>>(pred_name_2, state1, constant_yellow);
  auto green_light = std::make_shared<EqualityPredicate<std::string>>(pred_name_3, state1, constant_green);
  auto red_light2 = std::make_shared<EqualityPredicate<std::string>>(pred_name_4, state2, constant_red);
  auto yellow_light2 = std::make_shared<EqualityPredicate<std::string>>(pred_name_5, state2, constant_yellow);
  auto green_light2 = std::make_shared<EqualityPredicate<std::string>>(pred_name_6, state2, constant_green);

  std::vector<std::shared_ptr<statePredicate>> predicates = {red_light, yellow_light, green_light, red_light2, yellow_light2, green_light2};
  
  std::string trace_file = "trace_report_two_trafficLight_scarlet.trace";
  unsigned int number_of_mutants = 100;
  model_analyzer->generateScarletFile(trace_file, number_of_mutants, predicates);
}

TEST_F(TraceGeneratorTest, LogTraceReportToScarletArray)
{
  auto file_path = testFilesUtils->array_model();
  auto model_analyzer = std::make_unique<MutantAnalyzer>(file_path);

  auto constant_0 = std::make_shared<ConstantPredicate<int>>(0);
  auto constant_1 = std::make_shared<ConstantPredicate<int>>(1);
  auto constant_2 = std::make_shared<ConstantPredicate<int>>(2);
  auto constant_3 = std::make_shared<ConstantPredicate<int>>(3);
  auto array_0 = std::make_shared<VariablePredicate<int>>("array[0]");
  auto array_1 = std::make_shared<VariablePredicate<int>>("array[1]");
  auto array_2 = std::make_shared<VariablePredicate<int>>("array[2]");
  auto array_3 = std::make_shared<VariablePredicate<int>>("array[3]");
  std::string pred_name_1 = "array_0_eq_0";
  std::string pred_name_2 = "array_1_eq_0";
  std::string pred_name_3 = "array_2_eq_0";
  std::string pred_name_4 = "array_3_eq_0";
  std::string pred_name_5 = "array_1_eq_1";
  std::string pred_name_6 = "array_2_eq_2";
  std::string pred_name_7 = "array_3_eq_3";

  auto array_0_eq_0 = std::make_shared<EqualityPredicate<int>>(pred_name_1, array_0, constant_0);
  auto array_1_eq_0 = std::make_shared<EqualityPredicate<int>>(pred_name_2, array_1, constant_0);
  auto array_2_eq_0 = std::make_shared<EqualityPredicate<int>>(pred_name_3, array_2, constant_0);
  auto array_3_eq_0 = std::make_shared<EqualityPredicate<int>>(pred_name_4, array_3, constant_0);
  auto array_1_eq_1 = std::make_shared<EqualityPredicate<int>>(pred_name_5, array_1, constant_1);
  auto array_2_eq_2 = std::make_shared<EqualityPredicate<int>>(pred_name_6, array_2, constant_2);
  auto array_3_eq_3 = std::make_shared<EqualityPredicate<int>>(pred_name_7, array_3, constant_3);

  std::vector<std::shared_ptr<statePredicate>> predicates = {array_0_eq_0, array_1_eq_0, array_2_eq_0, array_3_eq_0, array_1_eq_1, array_2_eq_2, array_3_eq_3};
  auto trace_file = "trace_report_array_scarlet.trace";
  model_analyzer->generateScarletFile(trace_file, 10, predicates);
}

TEST_F(TraceGeneratorTest, LogTraceReportToScarletArrayWithGeneratedPredicates)
{
  auto file_path = testFilesUtils->array_model();
  auto model_analyzer = std::make_unique<MutantAnalyzer>(file_path);
  auto trace_file = "trace_report_array_scarlet_pred.trace";
  model_analyzer->generatePredicatesForScarletFile(trace_file, 10);
}

TEST_F(TraceGeneratorTest, LogTraceReportToScarletTrafficLightWithGeneratedPredicates)
{
  auto file_path = testFilesUtils->trafficLight_model_original();
  auto model_analyzer = std::make_unique<MutantAnalyzer>(file_path);
  auto trace_file = "trace_report_trafficLight_scarlet_pred.trace";
  model_analyzer->generatePredicatesForScarletFile(trace_file, 10);
}

TEST_F(TraceGeneratorTest, LogTraceReportToScarletTwoTrafficLightWithGeneratedPredicates)
{
  auto file_path = testFilesUtils->two_trafficLight_model_original();
  auto model_analyzer = std::make_unique<MutantAnalyzer>(file_path);
  auto trace_file = "trace_report_two_trafficLight_scarlet_pred.trace";
  model_analyzer->generatePredicatesForScarletFile(trace_file, 10);
}

TEST_F(TraceGeneratorTest, LogTraceReportToScarletMutexWithGeneratedPredicates)
{
  auto file_path = testFilesUtils->mutex_original();
  auto model_analyzer = std::make_unique<MutantAnalyzer>(file_path);
  auto trace_file = "trace_report_dijkstra_scarlet_pred.trace";
  unsigned int number_of_mutants = 50;
  model_analyzer->generatePredicatesForScarletFile(trace_file, number_of_mutants);
}

// TEST_F(TraceGeneratorTest, DiscardMutant)
// {
//   const TVL * tvl = nullptr;
//   auto file_path = testFilesUtils->array_model();
//   LTLClaimsProcessor::removeClaimFromFile(file_path);
//   auto original_loader = new promela_loader(file_path, tvl);
//   auto originalFSM = original_loader->getAutomata();
//   delete original_loader;
//   auto file_path_mutant = testFilesUtils->array_model_mutant();
//   LTLClaimsProcessor::removeClaimFromFile(file_path_mutant);
//   auto mutant_loader = std::make_unique<promela_loader>(file_path_mutant, tvl);
//   auto mutantFSM = mutant_loader->getAutomata();
//   auto res_formula = fsmExplorer::discardMutant(originalFSM, mutantFSM);
//   auto arr_0 = std::make_shared<VariableFormula>("array[0]");
//   auto arr_1 = std::make_shared<VariableFormula>("array[1]");
//   auto arr_2 = std::make_shared<VariableFormula>("array[2]");
//   auto arr_3 = std::make_shared<VariableFormula>("array[3]");
//   auto number_0 = std::make_shared<NumberConstant>(0);
//   auto number_1 = std::make_shared<NumberConstant>(1);
//   auto number_2 = std::make_shared<NumberConstant>(2);
//   auto number_3 = std::make_shared<NumberConstant>(3);
//   auto arr_0_eq_0 = std::make_shared<EqualsFormula>(arr_0, number_0);
//   auto arr_1_eq_1 = std::make_shared<EqualsFormula>(arr_1, number_1);
//   auto arr_2_eq_2 = std::make_shared<EqualsFormula>(arr_2, number_2);
//   auto arr_3_eq_0 = std::make_shared<EqualsFormula>(arr_3, number_0);
//   auto arr_3_leq_3 = std::make_shared<LargerEqualsFormula>(arr_3, number_3);
//   auto eventually = std::make_shared<FinallyFormula>(arr_3_leq_3);
//   auto next = std::make_shared<NextFormula>(eventually);
//   std::vector<std::shared_ptr<formula>> formulas = {arr_0_eq_0, arr_1_eq_1, arr_2_eq_2, arr_3_eq_0};
//   auto previousState = formulaUtility::combineFormulas(formulas, CombinationOperatorType::AND_Symbol);
//   auto implies_Formula = std::make_shared<ImpliesFormula>(previousState, next);
//   auto parentheses = std::make_shared<ParenthesisFormula>(implies_Formula);
//   auto expected_formula = std::make_shared<GloballyFormula>(parentheses);
//   std::cout << "Result: " << res_formula->toFormula() << std::endl;
//   std::cout << "Expected: " << expected_formula->toFormula() << std::endl;
//   ASSERT_TRUE(res_formula->isEquivalent(*expected_formula));
// }

TEST_F(TraceGeneratorTest, DiscardMutant_TrafficLight)
{
  const TVL * tvl = nullptr;
  auto file_path = testFilesUtils->trafficLight_model_original();
  LTLClaimsProcessor::removeClaimFromFile(file_path);
  auto original_loader = new promela_loader(file_path, tvl);
  auto originalFSM = original_loader->getAutomata();
  delete original_loader;
  auto file_path_mutant = testFilesUtils->trafficLight_model_mutant();
  LTLClaimsProcessor::removeClaimFromFile(file_path_mutant);
  auto mutant_loader = std::make_unique<promela_loader>(file_path_mutant, tvl);
  auto mutantFSM = mutant_loader->getAutomata();
  auto formula = fsmExplorer::discardMutant(originalFSM, mutantFSM);
  auto state_Var = std::make_shared<VariableFormula>("state");
  auto yellow = std::make_shared<VariableFormula>("yellow");
  auto red = std::make_shared<VariableFormula>("red");
  auto stateVar_eq_yellow = std::make_shared<EqualsFormula>(state_Var, yellow);
  auto stateVar_eq_red = std::make_shared<EqualsFormula>(state_Var, red);
  auto eventually = std::make_shared<FinallyFormula>(stateVar_eq_red);
  auto next = std::make_shared<NextFormula>(eventually);
  auto implies_Formula = std::make_shared<ImpliesFormula>(stateVar_eq_yellow, next);
  auto parentheses = std::make_shared<ParenthesisFormula>(implies_Formula);
  auto expected_formula = std::make_shared<GloballyFormula>(parentheses);
  std::cout << "Result: " << formula->toFormula() << std::endl;
  ASSERT_TRUE(formula->isEquivalent(*expected_formula));
}

// TEST_F(TraceGeneratorTest, RemoveCommonPrefixes)
// {
//   const TVL * tvl = nullptr;
//   auto file_path = testFilesUtils->array_model();
//   bool considerInternalVariables = false;
//   LTLClaimsProcessor::removeClaimFromFile(file_path);
//   auto original_loader = new promela_loader(file_path, tvl);
//   auto originalFSM = original_loader->getAutomata();
//   delete original_loader;
//   auto file_path_mutant = testFilesUtils->array_model_mutant();
//   LTLClaimsProcessor::removeClaimFromFile(file_path_mutant);
//   auto mutant_loader = std::make_unique<promela_loader>(file_path_mutant, tvl);
//   auto mutantFSM = mutant_loader->getAutomata();
//   auto trace_size = 12;
//   auto traceGen = std::make_unique<TraceGenerator>(originalFSM, mutantFSM);
//   auto report = traceGen->generateTraceReport(1, trace_size);
//   auto good_trace = *report->getGoodTraces().begin();
//   auto bad_trace = *report->getBadTraces().begin();
//   auto reduced_traces = formulaCreator::removeCommonPrefixes(good_trace, bad_trace);
//   auto states_removed_good = good_trace->size() - reduced_traces.first->size();
//   auto states_removed_bad = bad_trace->size() - reduced_traces.second->size();
//   auto transition_removed_good = good_trace->getTransitions().size() - reduced_traces.first->getTransitions().size();
//   auto transition_removed_bad = bad_trace->getTransitions().size() - reduced_traces.second->getTransitions().size();
//   // The reduced traces should be shorter than the original traces
//   ASSERT_TRUE(reduced_traces.first->size() < trace_size);
//   ASSERT_TRUE(reduced_traces.second->size() < trace_size);
//   // The number of states removed from the traces should be the same
//   ASSERT_TRUE(states_removed_good == states_removed_bad);
//   // The number of transitions removed from the traces should be the same
//   ASSERT_TRUE(transition_removed_good == transition_removed_bad);
//   // The two traces should have the same first state
//   auto first_state_good = reduced_traces.first->getStates().front();
//   auto first_state_bad = reduced_traces.second->getStates().front();
//   ASSERT_TRUE(first_state_good->isSame(first_state_bad.get(), considerInternalVariables));
//   // The next state should be different
//   auto second_state_good = reduced_traces.first->getStates().at(1);
//   auto second_state_bad = reduced_traces.second->getStates().at(1);
//   ASSERT_FALSE(second_state_good->isSame(second_state_bad.get(), considerInternalVariables));
// }

// TEST_F(TraceGeneratorTest, RemoveCommonPrefixes_TheTwoMethodShouldReturnTheSameResult)
// {
//   const TVL * tvl = nullptr;
//   auto file_path = testFilesUtils->array_model();
//   LTLClaimsProcessor::removeClaimFromFile(file_path);
//   auto original_loader = new promela_loader(file_path, tvl);
//   auto originalFSM = original_loader->getAutomata();
//   delete original_loader;
//   auto file_path_mutant = testFilesUtils->array_model_mutant();
//   LTLClaimsProcessor::removeClaimFromFile(file_path_mutant);
//   auto mutant_loader = std::make_unique<promela_loader>(file_path_mutant, tvl);
//   auto mutantFSM = mutant_loader->getAutomata();
//   auto trace_size = 12;
//   auto traceGen = std::make_unique<TraceGenerator>(originalFSM, mutantFSM);
//   auto report = traceGen->generateTraceReport(1, trace_size);
//   auto good_trace = *report->getGoodTraces().begin();
//   auto bad_trace = *report->getBadTraces().begin();
//   auto reduced_traces = formulaCreator::removeCommonPrefixes(good_trace, bad_trace);
//   report->removeCommonPrefixes(); // This method should return the same result as the previous method
//   good_trace = *report->getGoodTraces().begin();
//   bad_trace = *report->getBadTraces().begin();
//   ASSERT_TRUE(reduced_traces.first->size() == good_trace->size());
//   ASSERT_TRUE(reduced_traces.second->size() == bad_trace->size());
// }