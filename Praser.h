#ifndef PRASER_H
#define PRASER_H

#include "AST.h"
//#include "AsmCode.h"
using namespace std;

class Praser {
public:
	
	int varNum = 0; 	//临时变量个数
	int tempNum = 0; 	//变量个数
	int labelNum = 0; 	//标签号
	vector<string> codeList; //中间代码

//	AsmCode asmcode; 	//x86代码生成对象

	map<string, funcNode> funcPool;		//函数池（函数名--> 函数结点）
	map<string, string> labelPool;  	//标签映射关系（统一把标签映射为lable_i的形式）
	vector<struct Block> blockStack;	//维护的代码块栈

	struct AST* root;

	Praser(AST*);
	void praserInit();
	void praserAST(struct AST* node);
	void Praser::praser_fun_declaration(struct AST* node);
	void Praser::praser_param_list(struct AST* node,string funcName);
	void Praser::praser_param(struct AST* node, string funcName);
	void Praser::praser_compound_stmt(struct AST* node);
	void Praser::praser_var_declaration(struct AST *node);
	void Praser::praser_statement(struct AST* node);
	void Praser::praser_expression_stmt(struct AST *node);
	void Praser::praser_selection_stmt(struct AST* node);
	varNode Praser::praser_expression(struct AST* node);
	void Praser::praser_iteration_stmt(struct AST* node);
	void Praser::praser_return_stmt(struct AST* node);
	varNode Praser::praser_assignment_expression(struct AST* assign_exp);
	varNode Praser::praser_var(struct AST* primary_exp);
	varNode Praser::praser_simple_expression(struct AST* assign_exp);
	varNode Praser::praser_relop(varNode additive_expression_1, AST* relop, varNode additive_expression_2);
	varNode Praser::praser_additive_expression(struct AST* assign_exp);
	varNode Praser::praser_addop(varNode additive_expression, AST* addop, varNode term);
	varNode Praser::praser_term(struct AST* assign_exp);
	varNode Praser::praser_mulop(varNode term, AST* mulop, varNode factor);
	varNode Praser::praser_factor(struct AST* assign_exp);
	varNode Praser::praser_call(struct AST* call_exp);
	void Praser::praser_argument_expression_list(struct AST* node, string funcName);

	varNode Praser::createTempVar(string name, string type);
	struct varNode Praser::lookupNode(string name);
	void Praser::addCode(string str);
	void Praser::print_code();
	string Praser::getFuncRType();
	string Praser::getLabelName();
	string Praser::getNodeName(varNode node);
	string Praser::Gen_IR(string tempname, string op, varNode node1, varNode node2);
	void Praser::error(int line, string error);
	
};

#endif 