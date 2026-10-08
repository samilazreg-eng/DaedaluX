#pragma once
// This file captures the file paths of the promela files that are being tested.

#include "TestPaths.hpp"

#include <string>

class TestFilesUtils {
public:
  // locate is sharedTestFile or privateTestFile.
  explicit TestFilesUtils(std::string (*locate)(const std::string &)) : locate(locate) {}

  std::string array_model() { return locate("basic/array.pml"); }
  std::string flows_model() { return locate("basic/flows.pml"); }

  std::string array_model_original() { return locate("mutants/array.pml"); }
  std::string array_model_mutant() { return locate("mutants/array_mutant.pml"); }
  std::string array_model_mutant_alt() { return locate("mutants/array_mutant_alt.pml"); }

  std::string flows_model_original() { return locate("mutants/flows/flows.pml"); }
  std::string flows_model_mutant() { return locate("mutants/flows/flows_mutant.pml"); }

  std::string minepump_model_original() { return locate("mutants/minepump/minepump.pml"); }
  std::string minepump_model_mutant() { return locate("mutants/minepump/minepump_mutant.pml"); }

  std::string structure_model_original() { return locate("mutants/structure/structure.pml"); }
  std::string structure_model_mutant() { return locate("mutants/structure/structure_mutant.pml"); }

  std::string trafficLight_model_original() { return locate("mutants/trafficLight/trafficlight.pml"); }
  std::string two_trafficLight_model_original() { return locate("mutants/trafficLight/two_trafficlight.pml"); }
  std::string trafficLight_model_mutant() { return locate("mutants/trafficLight/trafficlight_mutant.pml"); }
  std::string trafficLight_model_mutant_alt() { return locate("mutants/trafficLight/trafficlight_mutant_alt.pml"); }

  std::string process_model_original() { return locate("mutants/3Process/threeProcess.pml"); }
  std::string process_model_mutant() { return locate("mutants/3Process/threeProcess_mutant.pml"); }

  std::string array_model_never() { return locate("mutants/array_never.pml"); };
  std::string array_mutant_never() { return locate("mutants/array_mutant_never.pml"); }

  std::string dijkstra_original() { return locate("mutants/dijkstra/original.pml"); }
  std::string mutex_original() { return locate("mutants/mutex/mutex.pml"); }
  std::string peterson_original() { return locate("mutants/peterson/original.pml"); }
  std::string leader_election_original() { return locate("mutants/leader_election/original.pml"); }

private:
  std::string (*const locate)(const std::string &);
};
