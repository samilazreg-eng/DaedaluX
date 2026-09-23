#include <assert.h>
#include <string>
#include <iostream>
#include <cstdlib>

#include <daedalux/promela/ast/expr/varExpr.hpp>

#include <daedalux/promela/symbol/vardef/varSymNode.hpp>
#include <daedalux/promela/symbol/vardef/variantSymNode.hpp>

#include <daedalux/promela/ast/expr/constExpr.hpp>

#include <daedalux/core/ast/visitor/astVisitor.hpp>
#include "daedalux/core/symbol/symTable.hpp"


exprVarRefName::exprVarRefName(const std::string& symName, int lineNb)
	: expr(astNode::E_VARREF_NAME, lineNb)
	, symName(symName)
	, sym(nullptr)
{
}

exprVarRefName::exprVarRefName(const std::string& symName, expr *index, int lineNb)
	: expr(astNode::E_VARREF_NAME, lineNb)
	, symName(symName)
	, sym(nullptr)
{
	assert(index);
	addChild("index", index);
}

exprVarRefName::exprVarRefName(const std::string& symName, symbol *sym, int lineNb)
	: expr(astNode::E_VARREF_NAME, lineNb)
	, symName(symName)
	, sym(sym)
{
	assert(sym);
	assert(getExprType() == symbol::T_NA);
	setExprType(sym->getType());
}

std::string exprVarRefName::getName(void) const {
	//assert(!sym || (symName == sym->getName()));
	return sym ? sym->getName() : symName;
}

void exprVarRefName::setIndex(expr* index) {
	eraseChild("index", index);
}

bool exprVarRefName::operator==(const astNode* other) const {
	auto cast = dynamic_cast<const exprVarRefName*>(other);
	assert(getName() != "");
	return cast != nullptr && getName() == cast->getName();
}

exprVarRefName::operator std::string() const {
	return getName() + (getIndex() ? "[" + std::string(*getIndex()) + "]" : "");
}

std::string exprVarRefName::getTypeDescr(void) const {
	return "Variable or field name (E_VARREF_NAME)";
}

symbol* exprVarRefName::exprVarRefName::getSymbol(void) const {
	return sym;
}

expr* exprVarRefName::getIndex(void) const {
	return dynamic_cast<expr*>(getChild("index"));
}

symbol* exprVarRefName::resolve(symTable *global, symTable *subField) {

	if (subField) {
		sym = dynamic_cast<symbol*>(subField->lookup(symName));
		//assert(sym);
	} else if (global) {
		do {
			sym = global->lookupGlobal(symName);
			global = global->prevSymTab();
		} while(!sym && global);
	} 
	
	if(!sym) {
		throw std::runtime_error("unknown symbol : " + symName + " at line " + std::to_string(lineNb));
	}

	assert(getExprType() == symbol::T_NA);
	setExprType(sym->getType());

	return sym;
}

expr* exprVarRefName::deepCopy(void) const {
	exprVarRefName* copy = new exprVarRefName(*this);
	copy->copyChildren(*this);
	return copy;
}

void exprVarRefName::acceptVisitor(ASTConstVisitor* visitor) const {
	visitor->visit(this);
}

void exprVarRefName::acceptVisitor(ASTVisitor* visitor) {
	visitor->visit(this);
}

/**********************************************************************************/

exprVarRef::exprVarRef(int lineNb, exprVarRefName *symRef, exprVarRef *subfieldsVar)
	: expr(astNode::E_VARREF, lineNb)
{
	assert(symRef);
	//assert(subfieldsVar);
	addChild("field", symRef);
	addChild("sub_field", subfieldsVar);
}

void exprVarRef::setVarRefName(exprVarRefName* varRefName) {
	eraseChild("field", varRefName);
}

exprVarRefName* exprVarRef::getVarRefName(void) const {
	return dynamic_cast<exprVarRefName*>(getChild("field"));
}

void exprVarRef::appendVarRef(exprVarRef* ref) {
	
	if(auto subField = getSubField()){
		subField->appendVarRef(ref);
	}
	addChild("sub_field", ref);
}

void exprVarRef::setSubField(exprVarRef* subField) {
	eraseChild("sub_field", subField);
}

bool exprVarRef::hasSubField(void) const {
	return getSubField() != nullptr;
}

exprVarRef * exprVarRef::getSubField(void) const {
	return dynamic_cast<exprVarRef*>(getChild("sub_field"));
}

exprVarRefName *exprVarRef::getField() const {
	return dynamic_cast<exprVarRefName*>(getChild("field"));;
}

symbol* exprVarRef::getFinalSymbol(void) const {
	return getSubField()? getSubField()->getFinalSymbol() : getField()->getSymbol();
}

symbol* exprVarRef::getFirstSymbol(void) const {
	return getField()->getSymbol();
}

exprVarRef::operator std::string() const {
	auto varRefName = getVarRefName();
	auto subfieldsVar = getSubField();
	assert(getVarRefName() /*&& getSubField()*/);
	return std::string(*varRefName) + (subfieldsVar ? "." + std::string(*subfieldsVar) : "");
}

unsigned int exprVarRef::assignMutables(const Mask& mask, unsigned int id) {
	if(mask.isPresent(type) && hasMutations()) 
		mId = ++id;
	return id;
}


std::string exprVarRef::getTypeDescr(void) const {
	return "Variable reference (E_VARREF)";
}

symbol* exprVarRef::resolve(symTable *global, symTable* subField) const {

	auto varRefName = getVarRefName();
	auto sym = varRefName->resolve(global, subField);

	//assert(sym);
	// Resolve subfields, but with the symbol table of the type
	if (sym && getSubField()) {
		
		//proc and utype are conceptually linked to refactor!
		auto cpxSym = dynamic_cast<complexSymNode*>(sym);

		auto fields = cpxSym->getSubSymTable();
		assert(fields);

		getSubField()->resolve(global, fields);
	}

	return sym;
}

symbol::Type exprVarRef::getExprType(void) const {
	if(!getVarRefName()->getSymbol())
		return symbol::T_NA;
	return getSubField()? getSubField()->getExprType() : getVarRefName()->getExprType();
}

bool exprVarRef::castToExprType(symbol::Type type) const {
	auto exprType = getExprType();
	switch(exprType){
		case symbol::T_BIT:
		case symbol::T_BYTE:
		case symbol::T_SHORT:
		case symbol::T_INT:
		case symbol::T_UNSGN:
			if(type == symbol::T_BIT
			|| type == symbol::T_BYTE
			|| type == symbol::T_SHORT
			|| type == symbol::T_INT
			|| type == symbol::T_UNSGN)
				return true;
			return false;
		case symbol::T_BOOL:
			if(type == symbol::T_BOOL)
				return true;
			return false;
		default:
			break;
	}
	return false;
}

std::vector<astNode*> exprVarRef::getMutations(void) const {
	auto sym = static_cast<varSymNode*>(getFinalSymbol());
	
	//std::cout << "expr var ref name : " << getVarRefName()->getName() << "\n";

	if(!sym || (!sym->getInitExpr() && sym->getMask() & symbol::WRITE_ACCESS))
		return std::vector<astNode*>();

	auto symList = getVarRefName()->getSymbol()->getSymTable()->getSymbols(getFinalSymbol());
	symList.erase(getVarRefName()->getSymbol());
	std::vector<astNode*> mutations;
	for(symbol* s: symList) {
		
		varSymNode* sCast = static_cast<varSymNode*>(s);
		//CAST BUG
		//varSymNode* sCast = dynamic_cast<varSymNode*>(s);
		//TODO : actual fix
		//assert(sCast);
		
		if(sCast) {
			if(sCast->getBound() > 1)
				for(unsigned int i = 0; i < sCast->getBound(); i++) {
					exprVarRefName* symRef = new exprVarRefName(s->getName(), sCast, lineNb);
					exprVarRef* newVar = new exprVarRef(lineNb, symRef);
					symRef->setIndex(new exprConst(i, lineNb));
					mutations.push_back(newVar);
				}
			else
				mutations.push_back(new exprVarRef(lineNb, new exprVarRefName(s->getName(), sCast, lineNb)));
		}
	}
	return mutations;
}

expr* exprVarRef::deepCopy(void) const {
	exprVarRef* copy = new exprVarRef(*this);
	copy->copyChildren(*this);
	return copy;
}

void exprVarRef::acceptVisitor(ASTConstVisitor* visitor) const {
	visitor->visit(this);
}

void exprVarRef::acceptVisitor(ASTVisitor* visitor) {
	visitor->visit(this);
}

/**********************************************************************************************************/

exprVar::exprVar(exprVarRef *varRef, int lineNb)
	: expr(astNode::E_EXPR_VAR, lineNb)
{
	assert(varRef);
	addChild("var_ref", varRef);
}

exprVar::exprVar(astNode::Type type, exprVarRef *varRef, int lineNb)
	: expr(type, lineNb)
{
	assert(varRef);
	addChild("var_ref", varRef);	
}

exprVarRef *exprVar::getVarRef(void) const {
	return dynamic_cast<exprVarRef*>(getChild("var_ref"));
}

//weird
exprVarRefName *exprVar::getVarRefName(void) const {
	return getVarRef()->getVarRefName();
}

exprVar::operator std::string() const {
	return *dynamic_cast<exprVarRef*>(getChild("var_ref"));
}

std::string exprVar::getTypeDescr(void) const {
	return "Variable reference wrapper (E_EXPR_VAR)";
}

symbol::Type exprVar::getExprType(void) const {
	return getVarRef()->getExprType();
}

bool exprVar::castToExprType(symbol::Type type) const {
	return getVarRef()->castToExprType(type);
}

symbol* exprVar::getFinalSymbol(void) const {
	return getVarRef()->getFinalSymbol();
}

symbol* exprVar::getFirstSymbol(void) const {
	return getVarRef()->getFirstSymbol();
}

expr* exprVar::deepCopy(void) const {
	exprVar* copy = new exprVar(*this);
	copy->copyChildren(*this);
	return copy;
}

void exprVar::acceptVisitor(ASTConstVisitor* visitor) const {
	visitor->visit(this);
}

void exprVar::acceptVisitor(ASTVisitor* visitor) {
	visitor->visit(this);
}

/****************************************************************************************************/

exprProjVar::exprProjVar(exprVarRef *varRef, exprVarRef* variant, int lineNb) 
	: exprVar(astNode::E_EXPR_PROJ_VAR, varRef, lineNb)
{
	assert(variant);
	addChild("variant", variant);
}

exprProjVar::operator std::string() const {
	return exprVar::operator std::string() + "{" + std::string(*getProjVarRefName()) + "}";
}

std::string exprProjVar::getTypeDescr(void) const {
	return "Projeted variable reference wrapper (E_EXPR_PROJ_VAR)";
}

exprVarRef* exprProjVar::getProjVarRef(void) const {
	return dynamic_cast<exprVarRef*>(getChild("variant"));
}

exprVarRefName* exprProjVar::getProjVarRefName(void) const {
	return getProjVarRef()->getVarRefName();
}

variantSymNode* exprProjVar::getVariantSymbol(void) const {
	return dynamic_cast<variantSymNode*>(getProjVarRefName()->getSymbol());
}

expr* exprProjVar::deepCopy(void) const {
	exprProjVar* copy = new exprProjVar(*this);
	copy->copyChildren(*this);
	return copy;
}