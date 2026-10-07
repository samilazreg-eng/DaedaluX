#ifndef FEATURED_STATE_DECORATOR_H
#define FEATURED_STATE_DECORATOR_H

#include <daedalux/core/semantic/variable/state/stateDecorator.hpp>

#include <cudd/cuddObj.hh>

class TVL;

// State
class featured : public stateDecorator {
public:
  // Creates the initial state by setting all variables' value in the payload. Does not set the payloadHash.
  featured(state * wrappee, const ADD & diagram, const TVL * tvl);

  featured(const featured * other);

  state * deepCopy(void) const override;

  ~featured() override;

  std::list<transition *> executables(void) const override;

  unsigned long hash(void) const override;

  byte compare(const state & s2) const override;

  byte compare(const state & s2, const ADD & features) const;

  byte compare(unsigned long s2Hash, const ADD & features) const;

  // Applying statements

  void apply(transition * trans) override;

  void print(void) const override;

  void printCSVHeader(std::ostream & out) const override;

  void printCSV(std::ostream & out) const override;

  ADD getFeatures(void) const;

  ADD getDiagram(void) const;

  bool constraint(const ADD & cst);

  void accept(stateVisitor * visitor) override;

public:
  ADD features;
  ADD diagram;
  ADD choices;
  ADD R;
  const TVL * tvl;
};

#endif