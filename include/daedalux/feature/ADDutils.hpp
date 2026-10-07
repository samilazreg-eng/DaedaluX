#ifndef ADD_UTILS_H
#define ADD_UTILS_H

#include <cudd/cuddObj.hh>

bool implies(const ADD& a, const ADD& b);

bool isTautology(const ADD& fct);

#endif
