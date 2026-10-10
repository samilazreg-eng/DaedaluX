#include <daedalux/mutants/spinRunner.hpp>

#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

bool spinRunner::spinInstalledFlag = false;

std::string exec(const char * cmd)
{
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

/// @brief The method checks if the model file satisfies the LTL formula using Spin. SPIN must be installed.
/// @param model_file the model file
/// @param f the LTL formula
/// @return True if the model satisfies the LTL formula, false otherwise
bool spinRunner::check(const std::string & model_file, const std::shared_ptr<formula> & f)
{
  // Check that the model file exists
  if (!std::filesystem::exists(model_file)) {
    std::cerr << "Error: Model file " << model_file << " does not exist." << std::endl;
    return false;
  }
  // Check that Spin is installed
  if (spinInstalledFlag == false && std::system("spin -V > /dev/null 2>&1") != 0) {
    std::cerr << "Error: Spin is not installed." << std::endl;
    return false;
  }
  else {
    spinInstalledFlag = true;
  }

  // Create a temporary file by copying the model file
  std::string temp_model_file = model_file + "_temp";
  std::filesystem::copy_file(model_file, temp_model_file, std::filesystem::copy_options::overwrite_existing);

  if (f) {
    // Remove any existing LTL claims from the model file
    LTLClaimsProcessor::removeClaimFromFile(temp_model_file);
    // Append the formula to the model file
    std::ofstream model_file_stream(temp_model_file, std::ios::app);
    model_file_stream << f->getDefinitionString() << std::endl;
    model_file_stream << "ltl f {" << std::endl;
    model_file_stream << "  " << f->promelaFormula() << std::endl;
    model_file_stream << "}" << std::endl;
    model_file_stream.close();
  }

  // Run Spin on the model file
  std::string command = "spin -run " + temp_model_file;
  std::string output = exec(command.c_str());

  // Check the output of Spin
  bool result = true;
  if (output.find("pan:1: acceptance cycle") != std::string::npos) {
    // The LTL property is violated and an acceptance cycle of the never claim was found
    result = false;
  }
  else if (output.find("pan:1: assertion violated") != std::string::npos) {
    // The LTL property is violated and an assertion was violated
    result = false;
  }
  else if (output.find("error: max search depth too small") != std::string::npos) {
    // The search depth was too small
    // TODO: Reconsider how to handle this case
    result = false;
  }
  else if (output.find("pan:1: invalid end state") != std::string::npos) {
    // A deadlock has been reached.
    result = false;
  }

  // Delete the temporary file
  std::filesystem::remove(temp_model_file);

  // Return the result of the Spin run
  return result;
}