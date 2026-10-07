#ifndef AST_TO_FSM_H
#define AST_TO_FSM_H

#include "../ast/visitor.hpp"

#include <stack>
#include <list>
#include <map>
#include <string>

#include <cudd/cuddObj.hh>

class symTable;

class fsm;
class fsmNode;
class fsmEdge;

class TVL;

class ASTtoFSM : public ASTConstVisitor {
public:
	ASTtoFSM(void);
	~ASTtoFSM() override;
	fsm* astToFsm(const symTable* symTab, const stmnt* program, const TVL* fm);

	//void visit(const stmnt* node) override;
	
	void visit(const stmntChanRecv* node) override;
	void visit(const stmntChanSnd* node) override;

	void visit(const stmntOpt* node) override;
	void visit(const stmntIf* node) override;
	void visit(const stmntDo* node) override;
	void visit(const stmntBreak* node) override;

	void visit(const stmntAction* node) override;

	void visit(const stmntGoto* node) override;
	void visit(const stmntLabel* node) override;
	void visit(const stmntFct* node) override;
	void visit(const stmntAtomic* node) override;
	void visit(const stmntDStep* node) override;
	void visit(const stmntSeq* node) override;

	void visit(const stmntAsgn* node) override;
	void visit(const stmntIncr* node) override;
	void visit(const stmntDecr* node) override;
	void visit(const stmntPrint* node) override;
	void visit(const stmntPrintm* node) override;
	void visit(const stmntAssert* node) override;
	void visit(const stmntExpr* node) override;
	void visit(const stmntElse* node) override;

	void visit(const varDecl* node) override;
	void visit(const chanDecl* node) override;
	void visit(const tdefDecl* node) override;
	void visit(const mtypeDecl* node) override;

	//void visit(const )

private:
	void _connect(fsmNode* target);
	void _label(fsmNode* node);
	fsmEdge* _looseEnd(const stmnt* node, bool owner = false);
	fsmEdge* _looseBreak(const stmnt* node);

	void _toFsm(const stmnt* node, bool label = true, bool connect = true, bool looseEnd = true, bool looseBreak = false);
	void _toFsmNode(const stmnt* node, bool label = true, bool connect = true, bool looseEnd = true, bool looseBreak = false);
	void _visitNext(const stmnt* node);

private:
	unsigned int flags;
	fsm* res;
	fsmNode *init;											 // The initial node
	fsmNode * current;
	//fsmNode *next;
	fsmNode* newNode;
	std::map<const stmnt*, fsmNode *> nodes;								 // List of ptFsmNode	- This list contains all nodes of the FSM in an arbitrary order.
	std::list<std::string> labels;
	std::map<std::string, fsmNode *> labeledNodes;			 // List of ptFsmNode	- This list is indexed by label and contains for each label the labelled node.
	std::list<fsmEdge *> trans;							 // List of fsmEdge	- This list contains all transitions of the FSM in an arbitrary order.
	std::list<fsmEdge *> looseEnds;						 // List of fsmEdge	- For model construction: contains those transitions that do not have an end state.
	std::list<fsmNode *> looseStarts;
	fsmEdge* prev;
	std::map<std::string, std::list<fsmEdge *>> looseGotos; // List of fsmEdge	- For model construction: contains those transitions (indexed by label name) that have yet to be connected to a state with this label.
	std::list<fsmEdge *> looseBreaks;						 // List of fsmEdge	- For model construction: contains those transitions that were generated because of a break statement and are waiting to be matched to a DO.
	//std::list<fsmEdge*> flowLooseEnds;
	//std::list<fsmEdge*> flowLooseBreaks;
	ADD elseFeatures;
	bool skip;
	const TVL* fm;
	ADD looseFeatures;
	bool hasElseFeatures;
};

#endif
