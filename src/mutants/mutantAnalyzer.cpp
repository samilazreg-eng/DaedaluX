
#include <daedalux/mutants/mutantAnalyzer.hpp>
#include <daedalux/formulas/formulaCreator.hpp>
#include <daedalux/algorithm/explore.hpp>
#include <daedalux/algorithm/fsmExplorer.hpp>
#include <daedalux/algorithm/ltlModelChecker.hpp>
#include <daedalux/promela/parser/promela_loader.hpp>
#include <daedalux/mutants/spinRunner.hpp>
#include <daedalux/algorithm/traceGenerator.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Constructor
MutantAnalyzer::MutantAnalyzer(const std::string & original_file_path, const std::string & property_file_path,
                               std::vector<std::string> mutant_file_paths)
    : original_file_path(original_file_path), property_file_path(property_file_path), mutant_file_paths(mutant_file_paths)
{
  if (!fileExists(original_file_path)) {
    std::cout << "Original file " << original_file_path << " does not exist" << std::endl;
    throw std::invalid_argument("Original file " + original_file_path + " does not exist");
  }
  if (!fileExists(property_file_path)) {
    std::cout << "Property file " << property_file_path << " does not exist" << std::endl;
    throw std::invalid_argument("Property file " + property_file_path + " does not exist");
  }
  for (auto mutant_file_path : mutant_file_paths) {
    if (!fileExists(mutant_file_path)) {
      std::cout << "Mutant file " << mutant_file_path << " does not exist" << std::endl;
      throw std::invalid_argument("Mutant file " + mutant_file_path + " does not exist");
    }
  }
}

MutantAnalyzer::MutantAnalyzer(const std::string & original_file_path, std::vector<std::string> mutant_file_paths)
    : original_file_path(original_file_path), mutant_file_paths(mutant_file_paths)
{
  if (!fileExists(original_file_path)) {
    std::cout << "Original file " << original_file_path << " does not exist" << std::endl;
    throw std::invalid_argument("Original file " + original_file_path + " does not exist");
  }
  for (auto mutant_file_path : mutant_file_paths) {
    if (!fileExists(mutant_file_path)) {
      std::cout << "Mutant file " << mutant_file_path << " does not exist" << std::endl;
      throw std::invalid_argument("Mutant file " + mutant_file_path + " does not exist");
    }
  }
}

MutantAnalyzer::MutantAnalyzer(const std::string & original_file_path, const std::string & property_file_path)
    : original_file_path(original_file_path), property_file_path(property_file_path)
{
  if (!fileExists(original_file_path)) {
    std::cout << "Original file " << original_file_path << " does not exist" << std::endl;
    throw std::invalid_argument("Original file " + original_file_path + " does not exist");
  }
  if (!fileExists(property_file_path)) {
    std::cout << "Property file " << property_file_path << " does not exist" << std::endl;
    throw std::invalid_argument("Property file " + property_file_path + " does not exist");
  }
}

MutantAnalyzer::MutantAnalyzer(const std::string & original_file_path) : original_file_path(original_file_path)
{
  if (!fileExists(original_file_path)) {
    std::cout << "Original file does not exist" << std::endl;
    throw std::invalid_argument("Original file does not exist");
  }
}

std::shared_ptr<formula> MutantAnalyzer::enhanceSpecification(unsigned int number_of_mutants)
{
  std::cout << "Enhance specification using mutation testing" << std::endl;
  // Create mutants
  createMutants(number_of_mutants);
  // Filter out bisimilar mutants
  // TODO implement - not sure how to do this yet - SAMI might be able to help with this

  // Kill mutants
  // Simply check whether the mutants are killed by the already specified properties
  auto [killed_mutants, surviving_mutants] = killMutantsSpin(nullptr);

  if (surviving_mutants.empty()) {
    std::cout << "All mutants are killed, no need to enhance the specification" << std::endl;
    return std::make_shared<BooleanConstant>(true);
  }

  // Analyze surviving mutants using trace generation and comparison with the original program  auto formulas =
  // analyzeMutants();
  auto formulas = analyzeMutants();
  std::vector<std::shared_ptr<formula>> formulas_vector = {};
  formulas_vector.reserve(formulas.size());

  // Print results
  for (auto [mutant, formula] : formulas) {
    std::cout << "Mutant " << mutant << " can be distinguished from the original program using the formula "
              << formula->toFormula() << std::endl;
    formulas_vector.push_back(formula);
  }

  // Combine formulas using the && operator
  auto combined_formula = formulaUtility::combineFormulas(formulas_vector, CombinationOperatorType::AND_Symbol);
  // Simplify the formula using the OWL tool

  return combined_formula;
}

bool checkPromelaModel(const std::string & file_path, std::shared_ptr<formula> f)
{
  ltlModelChecker * mc = new ltlModelChecker();
  bool model_correct = true;
  try {
    model_correct = mc->check(file_path);
  }
  catch (const std::exception & e) {
    std::cerr << "An error occurred while checking the model: ";
    std::cerr << e.what() << '\n';
    model_correct = false;
  }
  delete mc;
  return model_correct;
}

SurvivorKilledPair MutantAnalyzer::killMutantsSpin(std::shared_ptr<formula> f)
{
  auto result = checkMutants(spinRunner::check, f);
  return result;
}

/// @brief This function checks whether the mutants are killed by the already specified properties
/// @return Returns a pair of vectors, the first one containing the killed mutants and the second one containing the surviving
/// mutants
SurvivorKilledPair MutantAnalyzer::killMutants()
{
  auto result = checkMutants(checkPromelaModel, nullptr);
  return result;
}

void MutantAnalyzer::createMutants(unsigned int number_of_mutants, bool renameVariables)
{
  // Load promela file
  auto loader = std::make_unique<promela_loader>(original_file_path, nullptr);
  stmnt * program = loader->getProgram();
  unsigned int number_of_mutationPoints = program->assignMutables();
  // Each model gets its own folder next to it (the working directory for a bare file name), so models sharing a
  // directory never touch each other's mutants. Nothing is deleted: a rerun overwrites the files it writes.
  std::filesystem::path original(original_file_path);
  std::string mutant_folder = (original.parent_path() / (original.stem().string() + "_mutants")).string();
  std::filesystem::create_directories(mutant_folder);

  // Write original program to file so that it can be used by the mutation operators
  std::ofstream output;
  output.open(mutant_folder + "/original.pml");
  output << stmnt::string(program);
  output.close();

  // Generate mutants
  number_of_mutants = std::min(number_of_mutants, number_of_mutationPoints);
  for (unsigned int i = 1; i <= number_of_mutants; i++) {
    mutant_file_paths.push_back(createMutant(i, program, mutant_folder));
  }
}

std::map<std::string, std::shared_ptr<formula>> MutantAnalyzer::analyzeMutants()
{
  // Load original promela file
  auto original_loader = new promela_loader(original_file_path, nullptr);
  auto originalFSM = original_loader->getAutomata();
  delete original_loader;
  auto resultMap = std::map<std::string, std::shared_ptr<formula>>();
  // Loop through all mutants and compare them to the original one by one
  for (auto mutant_file_path : mutant_file_paths) {
    std::cout << "Analyzing mutant " << mutant_file_path << std::endl;
    auto mutant_loader = std::make_unique<promela_loader>(mutant_file_path, nullptr);
    auto mutantFSM = mutant_loader->getAutomata();
    auto formula = fsmExplorer::discardMutant(originalFSM, mutantFSM);
    // Add the formula to the map
    resultMap[mutant_file_path] = formula;
  }
  return resultMap;
}

/*
// Use threads to analyze mutants in parallel
std::mutex mtx; // used to synchronize access to resultMap

std::vector<std::thread> threads;
for (auto mutant_file_path : mutant_file_paths) {
    threads.push_back(std::thread([&](const std::string& path) {
        std::cout << "Analyzing mutant " << path << std::endl;
        auto mutant_loader = std::make_unique<promela_loader>(path, nullptr);
        auto mutantFSM = mutant_loader->getAutomata();
        auto formula = fsmExplorer::discardMutant(originalFSM, mutantFSM);
        mtx.lock();
        resultMap[path] = formula;
        mtx.unlock();
    }, mutant_file_path));
}

// Wait for all threads to finish
for (auto& t : threads) {
    t.join();
}
*/

std::string MutantAnalyzer::createMutant(int mutant_number, const stmnt * program, const std::string & mutant_folder)
{
  auto copy = program->deepCopy();
  astNode::mutate(copy, mutant_number);
  std::string name = "mutant_" + std::to_string(mutant_number);
  std::string file_name = mutant_folder + "/" + name + ".pml";
  std::ofstream output;
  output.open(file_name);
  output << stmnt::string(copy);
  output.close();
  delete copy;
  return file_name;
}

SurvivorKilledPair MutantAnalyzer::checkMutants(CheckFunction checkFunction, std::shared_ptr<formula> f)
{
  // Ensure that the original program is correct
  bool original_correct = checkFunction(original_file_path, f);
  if (!original_correct) {
    std::cerr << "The original program is incorrect" << std::endl;
    // Consider handling this case, e.g., throw an exception
  }

  auto killed_mutants = std::vector<std::string>();
  auto surviving_mutants = std::vector<std::string>();

  // One by one, check whether the already specified properties kill the mutants
  for (auto & mutant_file_path : mutant_file_paths) {
    bool model_correct = checkFunction(mutant_file_path, f);
    if (model_correct) {
      std::cout << "Mutant " << mutant_file_path << " is surviving" << std::endl;
      surviving_mutants.push_back(mutant_file_path);
    }
    else {
      std::cout << "Mutant " << mutant_file_path << " is killed" << std::endl;
      killed_mutants.push_back(mutant_file_path);
    }
  }

  // Remove the killed mutants from the list of mutants
  for (auto & killed_mutant : killed_mutants) {
    mutant_file_paths.erase(std::remove(mutant_file_paths.begin(), mutant_file_paths.end(), killed_mutant),
                            mutant_file_paths.end());
  }

  // Return the results
  return std::make_pair(killed_mutants, surviving_mutants);
}

void MutantAnalyzer::generateScarletFile(const std::string & file_path, unsigned int number_of_mutants,
                                         const std::vector<std::shared_ptr<statePredicate>> & predicates)
{
  createMutants(number_of_mutants);
  // Load original promela file
  auto original_loader = new promela_loader(original_file_path, nullptr);
  auto originalFSM = original_loader->getAutomata();
  delete original_loader;
  // Loop through all mutants and compare them to the original one by one

  auto positive_traces = std::unordered_set<std::shared_ptr<trace>>();
  auto negative_traces = std::unordered_set<std::shared_ptr<trace>>();
  auto trace_size = 20;
  auto appendLastStateIfDeadlock = true;

  for (auto mutant_file_path : mutant_file_paths) {
    auto mutant_loader = std::make_unique<promela_loader>(mutant_file_path, nullptr);
    auto mutantFSM = mutant_loader->getAutomata();

    auto trace_generator = std::make_unique<TraceGenerator>(originalFSM, mutantFSM);

    auto positive_trace = trace_generator->generatePositiveTrace(trace_size, appendLastStateIfDeadlock);
    auto negative_trace = trace_generator->generateNegativeTrace(trace_size, appendLastStateIfDeadlock);

    // Add the trace to the set of traces if it is a differentiating trace
    if (positive_trace->isDifferentiatingTrace()) {
      positive_traces.insert(positive_trace);
    }
    negative_traces.insert(negative_trace);
  }

  auto trace_report = std::make_unique<traceReport>(positive_traces, negative_traces);

  std::ofstream file(file_path);
  trace_report->printPredicatesEvaluation(file, predicates);
  file.close();
}

void MutantAnalyzer::generatePredicatesForScarletFile(const std::string & file_path, unsigned int number_of_mutants)
{
  createMutants(number_of_mutants);
  // Load original promela file
  auto original_loader = new promela_loader(original_file_path, nullptr);
  auto originalFSM = original_loader->getAutomata();
  delete original_loader;
  // Loop through all mutants and compare them to the original one by one

  auto positive_traces = std::unordered_set<std::shared_ptr<trace>>();
  auto negative_traces = std::unordered_set<std::shared_ptr<trace>>();
  auto trace_size = 20;
  auto appendLastStateIfDeadlock = true;

  for (auto mutant_file_path : mutant_file_paths) {
    auto mutant_loader = std::make_unique<promela_loader>(mutant_file_path, nullptr);
    auto mutantFSM = mutant_loader->getAutomata();

    auto trace_generator = std::make_unique<TraceGenerator>(originalFSM, mutantFSM);

    auto positive_trace = trace_generator->generatePositiveTrace(trace_size, appendLastStateIfDeadlock);
    auto negative_trace = trace_generator->generateNegativeTrace(trace_size, appendLastStateIfDeadlock);

    // Add the trace to the set of traces if it is a differentiating trace
    if (positive_trace->isDifferentiatingTrace()) {
      positive_traces.insert(positive_trace);
    }
    if (!negative_trace->isDifferentiatingTrace()) {
      std::cout << "Negative trace is not differentiating - consider adding more predicates or increasing the trace size"
                << std::endl;
    }
    negative_traces.insert(negative_trace);
  }

  auto trace_report = std::make_unique<traceReport>(positive_traces, negative_traces);
  auto predicates = trace_report->getPredicates();  // Get the predicates from the traces
  auto predicates_vector = std::vector<std::shared_ptr<statePredicate>>(predicates.begin(), predicates.end());
  std::ofstream file(file_path);
  trace_report->printPredicatesEvaluation(file, predicates_vector);
  file.close();
}