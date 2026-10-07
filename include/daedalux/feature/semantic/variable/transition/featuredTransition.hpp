#ifndef FEATURED_TRANSITION_H
#define FEATURED_TRANSITION_H

/*
 * Execution of FSMs
 * * * * * * * * * * * * * * * * * * * * * * * */

#include <daedalux/core/semantic/variable/transition/trans.hpp>

#include <cudd/cuddObj.hh>


// ProcessTransitions are returned by the executables() function
class featTransition : public transition {
public:
	featTransition(state* s, transition* wrappee, ADD featExpr);

	featTransition(const featTransition* other);
	
	~featTransition() override;

	ADD getFeatExpr(void) const;

	transition* deepCopy(void) const override;

	void accept(transitionVisitor* visitor) override;

	bool operator==(const transition* other) const override;

	float similarity(const transition* other) const override;

	void print(void) const override;

public:		//
	ADD features;
	transition* wrappee;
};

#endif
