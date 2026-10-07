#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <daedalux/feature/tvl.hpp>
#include <daedalux/feature/expToADD.hpp>

#include <daedalux/promela/symbol/vardef/utypeSymNode.hpp>
#include <daedalux/promela/symbol/typedef/tdefSymNode.hpp>

#include <daedalux/promela/ast/expr/constExpr.hpp>
#include <daedalux/promela/ast/expr.hpp>



Cudd* TVL::mgr = nullptr;
int TVL::maxId = 0;
std::map<std::string, int> TVL::featureIDMapping = std::map<std::string, int>();

TVL::TVL(void)
	: nbFeatures(0)
{
	if(!mgr)
		initBoolFct();
}

bool TVL::filterFeatureModel(const std::string& filename, const std::string& filter) const {
	FILE* fWorkingFile = fopen(filename.c_str(), "r");
	char* rootName;
	char buffer[1024];
	bool foundRoot = false;
	while(!foundRoot && fgets(buffer, 1024, fWorkingFile) != nullptr) {
		rootName = (char*)malloc(sizeof(char) * 32);
		if(!rootName) 
			printf("Out of memory (loading feature model).\n");
		if(sscanf(buffer, "root %s {", rootName) == 1) 
			foundRoot = true;
		else 
			free(rootName);
	}
	fclose(fWorkingFile);

	if(!foundRoot) { 
		printf("Could not find the root feature in your TVL file.  Please try again after moving it to the top of the file.\n");
		return false;
	}
	
	char* newLine = (char*)malloc(9 + strlen(rootName) + filter.size());
	sprintf(newLine, "\n%s {\n\t%s;\n}\n", rootName, filter.c_str());
	fWorkingFile = fopen(filename.c_str(), "a");
	fwrite(newLine, sizeof(char), 9 + strlen(rootName) + filter.size(), fWorkingFile);
	fclose(fWorkingFile);
	free(rootName);
	free(newLine);
	return true;
}

bool TVL::loadFeatureModel(const std::string& filename, const std::string& filter) {

	std::ifstream tvl;

	try {
		tvl.open(filename);

		if(std::filesystem::copy_file(filename, "__workingfile.tvl") == 0){
			if(!filter.empty())
				filterFeatureModel("__workingfile.tvl", filter);

			if(system("java -jar ./libs/tvl/TVLParser.jar -dimacs __mapping.tmp __clauses.tmp __workingfile.tvl") != 0) {
				//if(!keepTempFiles) remove("__workingfile.tvl");
				return 0;
			} else {
				return loadFeatureModelDimacs("__clauses.tmp", "__mapping.tmp");
				//if(!keepTempFiles) remove("__workingfile.tvl");
				//if(!keepTempFiles) remove("__mapping.tmp");
				//if(!keepTempFiles) remove("__clauses.tmp");
			}
		}

	} catch(const std::ifstream::failure& e) {
		std::cout<<"no tvl file exists"<<std::endl;
	}

	
	
	//printBool(getFeatureModelClauses);
	return false;
}

bool TVL::loadFeatureModelDimacs(const std::string& clauseFile, const std::string& mappingFile) {
	
	if(!dimacsFileToBool(clauseFile, featureModelClauses)) {
		printf("Could not find dimacs clauses file.\n");
		return false;
	}

	FILE* fMappingFile = fopen(mappingFile.c_str(), "r");
	char buffer[1024];
	int id;
	char* fname;
	if(fMappingFile == nullptr) {
		printf("Could not find dimacs mapping file.\n");
		return false;
	}
	
	while(fgets(buffer, 1024, fMappingFile) != nullptr) {
		fname = (char*)malloc(sizeof(char) * 128);
		if(!fname) printf("Out of memory (parsing dimacs file).\n");
		if(sscanf(buffer, "%d %s", &id, fname) == 2) {
			std::string strfname = fname;
			maxId = maxId >= id ? maxId : id;
			featureIDMapping[strfname] = id;
			nbFeatures++;
		}
		if(fname)
			free(fname);
	}

	fclose(fMappingFile);
	return true;

	//printBool(featureModelClauses);
}

bool TVL::dimacsFileToBool(const std::string& filePath, ADD& fm) {

	if(filePath.empty()) 
		return false;
	
	FILE* dimacsFile = fopen(filePath.c_str(), "r");
	char format[128];
	if(dimacsFile != nullptr) {
		fgets(format,128,dimacsFile);
		int currentNbr;
		BDD conjunction = mgr->bddOne();
		BDD disjunction = mgr->bddZero();
		while(fscanf(dimacsFile, "%d", &currentNbr) != EOF) {
			if(currentNbr == 0) {
				conjunction &= disjunction;
				disjunction = mgr->bddZero();
			} else { 
				auto absCurrentNbr = std::abs(currentNbr);
				BDD var = mgr->bddVar(absCurrentNbr);
				vars.push_back(var);
				if (currentNbr > 0)  
					var = mgr->bddVar(absCurrentNbr);
				else
					var = !mgr->bddVar(absCurrentNbr);
				disjunction |= var;
			}
		}
		fclose(dimacsFile);
		//printf("NB PRODUCTS : %lf\n", conjunction.CountMinterm(vars.size()));
		
		/*int i = 0;
		BDD curr = conjunction;
		//BDD res = _mgr->bddZero();
		while(!(curr.IsZero())) {
			BDD minterm = curr.PickOneMinterm(vars);
			//res |= minterm;
			curr &= !minterm;
			printf("%d\n", ++i);
		}

		printBool(conjunction.Add());
		printBool(curr.Add());
		*/
		fm = conjunction.Add();
		return true;

	}
	
	return false;
}

const ADD& TVL::getFeatureModelClauses() const {
    //printBool(getFeatureModelClauses);
	return featureModelClauses;
}

const std::vector<BDD>& TVL::getVars() const {
	return vars;
}

int TVL::getFeatureID(const std::string& name) {
	auto it = featureIDMapping.find(name);
	if(it != featureIDMapping.end())
		return it->second;
	return -1;
}

std::string TVL::getFeatureIDName(int id) {
	for(auto k : featureIDMapping){
		if(k.second == id)
			return k.first;
	}
	return "";
}

int TVL::getNbFeatures() const {
	return nbFeatures;
}

int TVL::createMapping(const std::string& name) {
	int id = getFeatureID(name);
	if(id != -1) {
		return id;
	}
	
	printf("%s\n",name.c_str());

	assert(featureIDMapping.find(name) == featureIDMapping.end());
	featureIDMapping[name] = ++maxId;


	return maxId;
}

bool TVL::hasFeature(const std::string& name) {
	return featureIDMapping.find(name) != featureIDMapping.end();
}

void TVL::initBoolFct(void) {
	mgr = new Cudd();
	//boolFctGarbageCollect("initBoolFct()");
	FILE* file = fopen("cudd_info.txt", "w");
	Cudd_PrintInfo(mgr->getManager(), file);
	fclose(file);
}

void TVL::deleteBoolFct(void) {
	delete mgr;
	mgr = nullptr;
}

void TVL::printInfo(void) const {
	FILE* file = fopen("cudd_info.txt", "a");
	Cudd_PrintInfo(mgr->getManager(), file);
	fclose(file);
}

BDD TVL::getFeature(const std::string& name) const {
	int id = getFeatureID(name.c_str());
	assert(id != -1);
	BDD res = mgr->bddVar(id);
	assert(res);
	return res;
}

ADD TVL::getFormula(utypeSymNode* features) const {
	//assert(spinMode);
	BDD minterm = mgr->addOne().BddPattern();

	for(auto f : features->getUType()->getFields()) {
		//printf("%s\n", features->name);
		BDD fbdd = getFeature(f->getName());
		assert(fbdd);

		auto cstExpr = dynamic_cast<exprConst*>(f->getInitExpr());
		if(cstExpr) {
			auto iVal = cstExpr->getCstValue();
			minterm &= iVal == 1? fbdd : !fbdd;
		}
	}

	return minterm.Add() * getFeatureModelClauses();
}

std::vector<ADD> TVL::getProducts(utypeSymNode* features) const {
	auto formula = getFormula(features);
	assert(formula);
	return getProducts(formula);
}

ADD TVL::getFormula(const expr* e) const {
	expToADD* visitor = new expToADD(this);
	e->acceptVisitor(visitor);
	return visitor->getFormula();
}

void TVL::printBool(const ADD& formula) {

	if(!formula) printf("All");
	//else if(isLogicZero(formula)) printf("None");
	else if(formula.IsOne()) printf("All");
	else {
		Cudd_SetStdout(formula.manager(), fopen("__printbool.tmp","w"));
		Cudd_PrintMinterm(formula.manager(), formula.getNode());
		fclose(Cudd_ReadStdout(formula.manager()));
		FILE * stream = fopen("__printbool.tmp", "r");
		char c = 'c';
		std::string feature;
		std::string op;
		int i = 0;
		byte nextliteral = 0;
		byte nextminterm = 0;
		printf("(");
		while(c != EOF && fscanf(stream, "%c", &c) != EOF) {
			if(c == '0') {
				if(nextliteral) op = (" & !");
				else if (nextminterm) op = ("|\n(!");
				else op = "!";
				feature = getFeatureIDName(i);
				printf("%s%s", op.c_str(), !feature.empty()? feature.c_str() : "_");
				nextliteral = 1;
				nextminterm = 0;
				i++;
			}
			else if (c == '1') {
				if(nextliteral) op = (" & ");
				else if (nextminterm) op = ("|\n(");
				else op = "";
				feature = getFeatureIDName(i);
				printf("%s%s", op.c_str(), !feature.empty()? feature.c_str() : "_");
				nextliteral = 1;
				nextminterm = 0;
				i++;
			}
			else if(c == '-'){
				if(nextliteral) op = (" & ?");
				else if (nextminterm) op = ("|\n(?");
				else op = "";
				feature = getFeatureIDName(i);
				//printf("%s%s", op.c_str(), !feature.empty()? feature : "_");
				nextliteral = 1;
				nextminterm = 0;
				i++;
			}
			else if(c == ' ') {
				printf(": \t");
				while(c != '\n' && fscanf(stream, "%c", &c) != EOF) {
					if(c != '\n') {
						printf("%c",c);
					}
				}
				printf("\n");
				nextliteral = 0;
				nextminterm = 1;
				i = 0;
			} else 
				assert(0);
		}
		printf(")\n");
		fclose(stream);

	}
		//Cudd_PrintMinterm(_manager, (DdNode *)formula);
}

void TVL::printBool(void) const {
	printBool(featureModelClauses);
}

std::string TVL::toString(const ADD& formula) {
	std::string res;
	if(!formula) res = "All";
	else if(formula.IsZero()) res = "None";
	else if(formula.IsOne()) res = "All";
	else {
		Cudd_SetStdout(formula.manager(), fopen("__printbool.tmp","w"));
		Cudd_PrintMinterm(formula.manager(), formula.getNode());
		fclose(Cudd_ReadStdout(formula.manager()));
		FILE * stream = fopen("__printbool.tmp", "r");
		char c = 'c';
		std::string feature;
		std::string op;
		int i = 0;
		byte nextliteral = 0;
		byte nextminterm = 0;
		while(c != EOF && fscanf(stream, "%c", &c) != EOF) {
			if(c == '0') {
				if(nextliteral) res += (" & !");
				else if (nextminterm) res += ("|(!");
				else res += "!";
				feature = getFeatureIDName(i);
				res += !feature.empty()? feature.c_str() : "_";
				nextliteral = 1;
				nextminterm = 0;
				i++;
			}
			else if (c == '1') {
				if(nextliteral) res += (" & ");
				else if (nextminterm) res += ("|(");
				else res += "!";
				feature = getFeatureIDName(i);
				res += !feature.empty()? feature.c_str() : "_";
				nextliteral = 1;
				nextminterm = 0;
				i++;
			}
			else if(c == '-'){
				if(nextliteral) op = (" & ?");
				else if (nextminterm) op = ("|(?");
				else op = "";
				feature = getFeatureIDName(i);
				//res += !feature.empty()? feature.c_str() : "_";
				nextliteral = 1;
				nextminterm = 0;
				i++;
			}
			else if(c == ' ') {
				while(c != '\n' && fscanf(stream, "%c", &c) != EOF);
				nextliteral = 0;
				nextminterm = 1;
				i = 0;
			} else 
				assert(0);
		}
		fclose(stream);
	}
	return res;
}

std::string TVL::toString(void) const {
	return toString(featureModelClauses);
}

std::vector<ADD> TVL::getProducts(const ADD& formula) const {
	std::vector<ADD> res;
	BDD current = getFeatureModelClauses().BddPattern();
	while(!(current.IsZero())) {
		BDD minterm = current.PickOneMinterm(getVars());
		current &= !minterm;
		res.push_back(minterm.Add() * formula);
	}
	return res;
}

int TVL::getNbProducts(void) const {
	return getProducts(featureModelClauses).size();
}

void TVL::printMinterms(const ADD& formula) const {

	//int index = 0;
	Cudd_SetStdout(formula.manager(), fopen("products", "w"));
	BDD current = getFeatureModelClauses().BddPattern();
	ADD res = mgr->addZero();
	while(!(current.IsZero())) {
		BDD minterm = current.PickOneMinterm(getVars());
		//res |= minterm.Add() * formula;
		current &= !minterm;
		ADD _minterm = minterm.Add() * formula;
		_minterm.PrintMinterm();
		//printf("product %d ", ++index);
		//printBool(minterm.Add() * formula);
	}
	//printBool(res);
	fclose(Cudd_ReadStdout(formula.manager()));
}

Cudd* TVL::getMgr(void) {
	return mgr;
}
