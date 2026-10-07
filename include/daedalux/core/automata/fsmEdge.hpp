#ifndef EDGE_H
#define EDGE_H
/*
 * FINITE STATE MACHINES (FSMs)
 * * * * * * * * * * * * * * * * * * * * * * * */

#include <string>
#include <cudd/cuddObj.hh>

class astNode;

class fsm;
class fsmNode;

class fsmEdge
{

public:
	fsmEdge(fsmNode *source, const astNode *expression, int lineNb, bool owner = false);
	fsmEdge(fsmNode *source, fsmNode *target, const astNode *expression, int lineNb, bool owner = false);
	fsmEdge(fsmNode *source, fsmNode *target, const astNode *expression, ADD features, double prob, int lineNb, bool owner = false);

	~fsmEdge();
	fsmNode* setTargetNode(fsmNode *target);
	fsmNode* setSourceNode(fsmNode *source);

	int getLineNb(void) const;
	void setLineNb(int line);
	bool hasFeatures(void) const;
	const ADD& getFeatures(void) const;
	void setFeatures(const ADD& features);
	
	// bool project(const ADD& features);
	//getExpr?
	
	const astNode *getExpression(void) const;
	double getProbability(void) const;
	void setExpression(const astNode *expression);
	fsmNode *getSourceNode(void) const;
	fsmNode *getTargetNode(void) const;

	operator std::string(void) const;
	bool operator==(const fsmEdge& other) const;
	float similarity(const fsmEdge& other) const;


public:
	fsm* parent;
	int lineNb;
	ADD features;
	fsmNode *source;
	const astNode * expression;
	fsmNode *target;
	double prob;
	bool owner;
};

#endif
