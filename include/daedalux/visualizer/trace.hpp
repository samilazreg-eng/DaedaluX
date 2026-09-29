#pragma once

#include <cstdint>
#include <iostream>
#include <memory> // Include for smart pointers
#include <sstream>
#include <string>
#include <vector>

#include <daedalux/core/semantic/variable/state/stateVar.hpp>
#include <daedalux/core/semantic/variable/transition/trans.hpp>
#include <daedalux/formulas/predicates/statePredicate.hpp>

class trace {
private:
  std::vector<std::shared_ptr<transition>> transitions = std::vector<std::shared_ptr<transition>>();
  std::vector<std::shared_ptr<state>> states = std::vector<std::shared_ptr<state>>();
  bool isDifferentiating = false;
  int differentiating_index = -1;

public:
  trace();
  trace(const trace & other);
  trace(const trace * other);
  trace(std::vector<std::shared_ptr<transition>> transitions, std::vector<std::shared_ptr<state>> states);
  explicit trace(std::vector<std::shared_ptr<state>> states);
  
  void setDifferentiatingTrace(bool isDifferentiating) { this->isDifferentiating = isDifferentiating; }
  bool isDifferentiatingTrace() const { return this->isDifferentiating; }
  std::string projectTrace(const std::vector<std::shared_ptr<statePredicate>> & predicates);

  std::vector<std::shared_ptr<statePredicate>> getPredicates() const;

  ~trace();

  void findDistinguishingFormula(const std::shared_ptr<trace> t);

  bool containState(const std::shared_ptr<state> s) const;
  std::vector<std::shared_ptr<transition>> getTransitions() const { return this->transitions; }
  std::vector<std::shared_ptr<state>> getStates() const { return this->states; }

  void addTransition(std::shared_ptr<transition> t) { this->transitions.push_back(t); }
  void addState(std::shared_ptr<state> s, bool differentiate = false)
  {
    if (differentiate && !this->isDifferentiating) {
      // If we are differentiating and we are not already differentiating, then we start differentiating
      this->isDifferentiating = true;
      this->differentiating_index = this->states.size();
    }
    this->states.push_back(s);
  }

  void removeTransitionAt(int index) { this->transitions.erase(this->transitions.begin() + index); }
  void removeStateAt(int index) { this->states.erase(this->states.begin() + index); }

  size_t size() const { return this->states.size(); }

  void addTransitions(const std::vector<std::shared_ptr<transition>> & Ts)
  {
    for (auto t : Ts) {
      this->addTransition(t);
    }
  }
  void addStates(const std::vector<std::shared_ptr<state>> & Ss)
  {
    for (auto s : Ss) {
      this->addState(s);
    }
  }

  void addTrace(const trace * other)
  {
    this->addTransitions(other->getTransitions());
    this->addStates(other->getStates());
  }

  /// @brief This methods prints the trace to the standard output.
  /// @details This method prints the first state, then all of the deltas between the states, and finally the last state.
  void print() const
  {
    if (this->states.empty()) {
      std::cout << "Empty trace" << std::endl;
      return;
    }
    std::cout << "Printing trace" << std::endl;
    std::cout << "----------------" << std::endl;
    std::cout << "Number of states: " << this->states.size() << std::endl;
    auto previous_state = this->states.front();
    std::cout << "Initial state" << std::endl;
    previous_state->print();
    bool considerInternalVariables = true;
    for (long unsigned int i = 1; i < this->states.size(); i++) {
      auto current_state = this->states[i];
      previous_state->printDelta(current_state.get(), considerInternalVariables);
      previous_state = current_state;
    }
    // Print the last state
    std::cout << "Last state" << std::endl;
    previous_state->print();
    std::cout << "----------------" << std::endl;
  }

  void printCSV(std::ostream & out) const
  {
    auto start = this->states.front();
    start->printCSVHeader(out);
    out << std::endl;
    for (auto st : this->states) {
      st->printCSV(out);
      out << std::endl;
    }
  }

  std::string to_string() const
  {
    std::stringstream s = std::stringstream();
    for (auto st : this->states) {
      st->printCSV(s);
    }
    return s.str();
  }

  friend bool operator==(const trace & lhs, const trace & rhs)
  {
    auto l_states = lhs.getStates();
    auto r_states = rhs.getStates();
    int lhs_number_of_states = l_states.size();
    int rhs_number_of_states = r_states.size();
    auto lhs_number_of_transitions = lhs.getTransitions().size();
    auto rhs_number_of_transitions = rhs.getTransitions().size();
    if (lhs_number_of_states != rhs_number_of_states || lhs_number_of_transitions != rhs_number_of_transitions) {
      return false;
    }
    else {
      if (lhs_number_of_states == 0 || lhs_number_of_transitions == 0) {
        return true;
      }
    }

    bool considerInternalVariables = true;
    for (int i = 0; i < lhs_number_of_states; i++) {
      if (!l_states[i]->isSame(r_states[i].get(), considerInternalVariables)) {
        return false;
      }
    }
    // TODO: Fix this
    // for (int i = 0; i < lhs.getTransitions().size(); i++) {
    //   if (*l_transitions[i] != *r_transitions[i]) {
    //     return false;
    //   }
    // }
    return true;
  }

  // std::map<std::string, std::pair<int, int>> getMinMaxValues() const;

  friend bool operator!=(const trace & lhs, const trace & rhs) { return !(lhs == rhs); }
};

template <typename... Args> size_t hash_all(const Args &... args)
{
  uint64_t hash = 0xc3a5c85c97cb3127ULL;
  auto hash_mix = [&hash](uint64_t v) {
    hash += v;
    hash ^= hash >> 33;
    hash *= 0xff51afd7ed558ccdULL;
    hash ^= hash >> 33;
    hash *= 0xc4ceb9fe1a85ec53ULL;
    hash ^= hash >> 33;
  };
  (hash_mix(std::hash<Args>{}(args)), ...);
  return hash;
}

template <> struct std::hash<trace> {
  size_t operator()(const trace & t) const noexcept
  {
    auto transitions = t.getTransitions();
    uint64_t hash = 0;
    for (auto tr : transitions) {
      hash = hash_all(tr->src->hash(), tr->dst->hash(), tr->action, tr->prob, hash);
    }
    for (auto s : t.getStates()) {
      hash = hash_all(s, hash);
    }
    return hash;
  }
};