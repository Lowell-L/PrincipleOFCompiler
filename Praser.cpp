#include "Praser.h"

using namespace std;

Praser::Praser(AST* root) {//构造函数
	this->root = root;
	praserInit();
	printf("---------------------debug_info_end---------------------------\n");
	print_code();
	printf("---------------------inner_code_end---------------------------\n");
	//asmcode.CodeGen(codeList);
	//asmcode.printCode();
	printf("--------------------objcoed_gen_end---------------------------\n");
}

void Praser::praserInit() { //初始化
	Block wholeBlock; 		//创建整个程序的代码块信息（全局）
	blockStack.push_back(wholeBlock);  
	
	//加入内置函数 input：读入一个int 并返回  output：打印一个int值
	funcNode output;
	output.name = "output";
	output.rtype = "void";
	varNode pnode;
	pnode.type = "int";
	output.paralist.push_back(pnode);
	funcPool.insert({ "output", output });

	funcNode input;
	input.name = "input";
	input.rtype = "input";
	funcPool.insert({"input",input});

	praserAST(root);		//开始分析语法树

	blockStack.pop_back();
}

void Praser::praserAST(struct AST* node) {
	if (node == NULL)
		return;
	else if (node->type == "fun-declaration") { //解析函数
		praser_fun_declaration(node);
		return;
	}
	else if (node->type == "var-declaration") {	//解析变量
		praser_var_declaration(node);
		return;
	}
	else if (node->type == "statement") {		//解析语句
		praser_statement(node);
		return;
	}  
	else{	//继续向下递归分析
		praserAST(node->left);
		praserAST(node->right);
	}
}

//规则 6、7	解析函数定义
void Praser::praser_fun_declaration(struct AST* node) {
	AST* type_specifier;			//返回类型结点
	type_specifier = node->left;
	
	AST* ID = node->left->right;	//函数名称结点

	AST* compound_stmt = ID->right->right->right->right;
	
	string funcType = type_specifier->left->text;	//返回值类型

	string funcName = ID->text;				//函数名
	
	funcNode declarFunc;					//函数结点

	if (funcPool.find(funcName) != funcPool.end()) {
		//该函数名在funcPool中找到，函数重复定义
		error(ID->line, "函数重复定义：" + funcName );
	}

	Block funBlock;
	funBlock.isfunc = true;
	funBlock.func.name = funcName;
	funBlock.func.rtype = funcType;
	//将函数记录在块内并添加到函数池
	blockStack.push_back(funBlock);
	funcPool.insert({funcName,funBlock.func});

	cout<<"Debug:成功创建Block和插入了funcPool,函数："<<funcName<<endl;

	//中间代码
	addCode("FUNC " + funcName + ":");

	//获取函数形参列表
	if(ID->right->right->left->type == "param-list"){
		cout<<"Debug:开始解析一个带参数（param-list）的 function"<<endl;
		praser_param_list(ID->right->right->left, funcName);
		cout<<"Debug:参数解析完毕"<<endl;
	}
	else
	{
		//参数列表 VOID 无需解析参数列表
	}
	
	//此时函数池中的func已经添加了参数列表
	funcNode func = funcPool[funcName];

	//更新Block中func的参数列表
	funBlock.func = func;
	//分析函数的正文
	praser_compound_stmt(compound_stmt->left->right);

	//函数结束后，弹出相应的block
	blockStack.pop_back();

}

//规则 8	多参数处理
void Praser::praser_param_list(struct AST* node,string funcName) {
	if (node->left->type == "param-list") {
		praser_param_list(node->left, funcName);
		praser_param(node->left->right->right, funcName);
	}
	else if (node->left->type == "param") {
		praser_param(node->left,funcName);
	}
}

//规则 9	获取单个形参内容
void Praser::praser_param(struct AST* node, string funcName) {

	AST* type_specifier = node->left;	//参数类型结点
	AST* ID = node->left->right;		//参数名称结点

	string typeName = type_specifier->left->text;		//参数类型
	string varName = ID->text;		//参数名称

	varNode newnode;
	newnode.name = varName;
	newnode.type = typeName;
	newnode.num = varNum++;			

	blockStack.back().func.paralist.push_back(newnode);	//变量加入代码池中的函数块的形参列表中
	cout<<"Debug:成功将参数:"<<varName<<"添加到:"<<funcName<<endl;

	funcPool[funcName].paralist.push_back(newnode);		//变量加入函数池中的函数块的形参列表中
	
	blockStack.back().varMap.insert({varName,newnode});	//将函数的形参添加到当前块的变量池中
	addCode("PARAM_IN var" + inttostr(newnode.num));	
}

//规则10	处理语句块
void Praser::praser_compound_stmt(struct AST* node) {
	//继续分析处理compound_stmt
	//对于{}块中的代码，创建一个新的块，新的命名空间
	Block newblock;
	blockStack.push_back(newblock);
	
	praserAST(node);  			//local_declarations
	//praserAST(node->right);		//statement_list

	blockStack.pop_back();
}

//规则 4	解析变量声明
void Praser::praser_var_declaration(struct AST *node) {
	
	cout<<"Debug:开始解析 praser_var_declaration"<<endl;

	AST* type_specifier = node->left;	//参数类型结点
	AST* ID = node->left->right;		//参数名称结点

	string typeName = type_specifier->left->text;		//参数类型
	string varName = ID->text;		//参数名称
			
	if ( blockStack.back().varMap.find(varName) == blockStack.back().varMap.end() ) {
		varNode newnode;
		newnode.name = varName;
		newnode.type = typeName;
		newnode.num = varNum++;
		blockStack.back().varMap.insert({varName, newnode});	//变量加入代码池变量列表中
	}
	else{
		error(node->left->right->line, "变量重复定义");
	}

	return ;
}

//规则13	解析statement语句
void Praser::praser_statement(struct AST* node) {
	struct AST* next = node->left;
	if(node->left->type == "expression-stmt") {//单个;或者 expression ';'
		praser_expression_stmt(node->left);
	}
	else if (node->left->type == "compound-stmt") {//复合语句  {。。。} 
		praser_compound_stmt(node->left);
	}
	else if (node->left->type == "selection-stmt") { // if语句
		praser_selection_stmt(node->left);
	}
	else if (node->left->type == "iteration-stmt") {//迭代语句 while
		praser_iteration_stmt(node->left);
	}
	else if (node->left->type == "return-stmt") { //return
		praser_return_stmt(node->left);
	}
}

//规则 14	表达式语句
void Praser::praser_expression_stmt(struct AST *node) {
	if (node->left->type == "expression") {
		praser_expression(node->left);
	}
	else if(node->left->type == ";"){
		//空语句 单个;
	}
}	

//规则 18	解析expression
varNode Praser::praser_expression(struct AST* node) {
	if (node->left->type == "var") {
		return praser_assignment_expression(node->left);
	}
	else if(node->left->type == "simple_expression") {
		return praser_simple_expression(node->left);
	}
}

//规则 15	解析if语句
void Praser::praser_selection_stmt(struct AST* node) {
	cout<<"Debug:142 开始解析IF语句："<<node->left->type<<endl;
	if (node->left->type == "if") {
		if (node->left->right->right->right->right->right == NULL) {
			//添加一个新的block  没有else语句
			Block newblock;
			blockStack.push_back(newblock);

			AST* expression = node->left->right->right;
			varNode exp_rnode = praser_expression(expression);
			AST* statement = node->left->right->right->right->right;

			string label1 = getLabelName();
			string label2 = getLabelName();

			addCode("IF " + getNodeName(exp_rnode) + " != 0" + " GOTO " + label1);
		
			addCode("GOTO " + label2);
			addCode(label1 + ":");

			praser_statement(statement);
			
			addCode(label2 + ":");

			//弹出添加的block
			blockStack.pop_back();

		}
		else if (node->left->right->right->right->right->right->type == "else") {
			//添加一个新的block
			Block newblock1;
			blockStack.push_back(newblock1);

			AST* expression = node->left->right->right;
			varNode exp_rnode = praser_expression(expression);
			AST* statement1 = node->left->right->right->right->right;
			AST* statement2 = node->left->right->right->right->right->right->right;

			string label1 = getLabelName();
			string label2 = getLabelName();
			string label3 = getLabelName();


			addCode("IF " + getNodeName(exp_rnode) + " != 0 "  + " GOTO " + label1);
			addCode("GOTO " + label2);
			addCode(label1 + ":");

			praser_statement(statement1);
			
			addCode("GOTO " + label3);
			//弹出添加的block
			blockStack.pop_back();

			//else
			addCode(label2 + ":");

			Block newblock2;
			blockStack.push_back(newblock2);

			praser_statement(statement2);

			addCode(label3 + ":");

			blockStack.pop_back();

		}
	}
	else{
		cout<<"Debug:dsa praser_selection_stmt,未定义的语句："<<node->left->type<<endl;
	}
}

//规则 16	解析while语句
void Praser::praser_iteration_stmt(struct AST* node) {
	if (node->left->type == "While") {

		//添加一个新的block
		Block newblock;
		blockStack.push_back(newblock);

		struct AST* expression = node->left->right->right;
		struct AST* statement = node->left->right->right->right->right;

		string label1 = getLabelName(); //判断语句的开始
		string label2 = getLabelName(); //循环体语句
		string label3 = getLabelName(); //循环外语句

		blockStack.back().continueLabelname = label1;
		blockStack.back().breakLabelname = label3;
		
		addCode(label1 + ":");

		varNode var = praser_expression(expression);

		addCode("IF " + getNodeName(var) + " != 0"  + " GOTO " + label2);
	
		addCode("GOTO " + label3);
		addCode(label2 + ":");

		praser_statement(statement);

		addCode("GOTO " + label1);
		addCode(label3 + ":");
		
		//弹出添加的block
		blockStack.pop_back();
	}
	else{
		cout<<"Debug:dsa praser_iteration_stmt,未定义的语句："<<node->left->type<<endl;
	}
}

//规则 17	解析return语句
void Praser::praser_return_stmt(struct AST* node) {
	if (node->left->type == "RETURN") {
		string funcType = getFuncRType();
		if (node->left->right->type != ";") {//return expression
			varNode rnode = praser_expression(node->left->right);
			if(rnode.num<0){
				addCode("RETURN " + rnode.name );
			}else{
				addCode("RETURN var" + inttostr(rnode.num) );
			}
			
			if (rnode.type != funcType) {
				cout <<"Dubug:123 返回类型不匹配"<< rnode.type <<"and"<<funcType <<endl;
				error(node->left->right->line, "You must return a value with type:" + funcType);
			}
		}
		else if (node->left->right->type == ";"){	//return ;
			addCode("RETURN");
			if (funcType != "void") {
				error(node->left->right->line, "You must return a value with type:" + funcType);
			}
		}
		else {
			cout<<"Dubug:dqw 未知的错误"<<endl;
		}
	}
}

//规则 18.1	赋值语句解析
varNode Praser::praser_assignment_expression(struct AST* assign_exp) {	//返回变量节点
	cout<<"Debug:开始解析赋值语句:"<<assign_exp->type<<endl;
	
	struct AST* primary_exp=assign_exp;
	string op = primary_exp->right->type;
	struct AST* next_assign_exp = primary_exp->right->right;

	varNode node1 = praser_var(primary_exp);
	varNode node2 = praser_expression(next_assign_exp);
	varNode node3;

	if (op == "=") {
		node3 = node2;
	}

	if( node3.num < 0){	//是临时变量 如 a=1+1;
		addCode("var"+ inttostr(node1.num) +" = "+ node3.name);
	}
	else{				//是已知变量 如 a=b;
		addCode("var"+ inttostr(node1.num) +" = var"+inttostr(node3.num));
	}		

	return node1;		
}

//规则 19
varNode Praser::praser_var(struct AST* primary_exp) {
	if (primary_exp->left->type == "ID") {
		string content = primary_exp->left->text;
		varNode rnode = lookupNode(content);
		if (rnode.num < 0) {
			error(primary_exp->left->line, "Undefined variable " + content);
		}
		return rnode;
	}
}

//规则 20
varNode Praser::praser_simple_expression(struct AST* assign_exp){
	cout<<"Debug:czxc 开始解析表达式:"<<assign_exp->type<<endl;
	
	if(assign_exp->left->right == NULL)
		return praser_additive_expression(assign_exp->left);
	else{
		varNode additive_expression_1 = praser_additive_expression(assign_exp->left);
		varNode additive_expression_2 = praser_additive_expression(assign_exp->left->right->right);
		return praser_relop(additive_expression_1, assign_exp->left->right, additive_expression_2);
	}
}

//规则 21	
varNode Praser::praser_relop(varNode additive_expression_1, AST* relop, varNode additive_expression_2){
	if(relop->type == "EQ" || relop->type == "NE" || relop->type == "BE" || relop->type == "AE" || 
		relop->type == "<" || relop->type == ">")
	{
		string op;
		if (relop->type == "EQ")
			op = "==";
		else if(relop->type == "NE")
			op = "!=";
		else if(relop->type == "BE")
			op = "<=";
		else if(relop->type == "AE")
			op = ">=";
		else op = relop->type;

		if (additive_expression_1.type != additive_expression_1.type) {
			error(relop->line, "类型不匹配！");
		}

		string tempname = "temp" + inttostr(tempNum);
		++tempNum;
		varNode newnode = createTempVar(tempname, "int");
		blockStack.back().varMap.insert({ tempname,newnode});
		addCode(Gen_IR(tempname, op, additive_expression_1, additive_expression_1));

		return newnode;
	}
}

//规则 22	
varNode Praser::praser_additive_expression(struct AST* assign_exp){
	if(assign_exp->left->right == NULL)
		return praser_term(assign_exp->left);
	else{
		varNode additive_expression = praser_additive_expression(assign_exp->left);
		varNode term = praser_term(assign_exp->left->right->right);
		return praser_addop(additive_expression, assign_exp->left->right, term);
	}
}

//规则 23
varNode Praser::praser_addop(varNode additive_expression, AST* addop, varNode term){
	if(addop->type == "+" || addop->type == "-"){
		if (additive_expression.type != term.type) {
			cout<<"Debug:不同的参数类型（+）："<< additive_expression.type << "和" <<term.type<<endl;
			error(addop->line, "Different type for two variables.");
		}

		string tempname = "temp" + inttostr(tempNum);
		++tempNum;
		varNode newnode = createTempVar(tempname, additive_expression.type);
		blockStack.back().varMap.insert({ tempname,newnode});

		addCode(Gen_IR(tempname,addop->type, additive_expression, term));
		return newnode;
	}
}

//规则 24
varNode Praser::praser_term(struct AST* assign_exp){
	if(assign_exp->left->right == NULL)
		return praser_factor(assign_exp->left);
	else{
		varNode term = praser_term(assign_exp->left);
		varNode factor = praser_factor(assign_exp->left->right->right);
		return praser_mulop(term, assign_exp->left->right, factor);
	}
}

//规则 25
varNode Praser::praser_mulop(varNode term, AST* mulop, varNode factor){
	if(mulop->type == "*" || mulop->type == "/"){
		if (term.type != factor.type) {
			cout<<"Debug:不同的参数类型（+）："<< term.type << "和" <<factor.type<<endl;
			error(mulop->line, "Different type for two variables.");
		}

		string tempname = "temp" + inttostr(tempNum);
		++tempNum;
		varNode newnode = createTempVar(tempname, term.type);
		blockStack.back().varMap.insert({ tempname,newnode});

		addCode(Gen_IR(tempname,mulop->type, term, factor));
		return newnode;
	}
}

//规则 26
varNode Praser::praser_factor(struct AST* assign_exp){

	if(assign_exp->left->type == "("){
		return praser_expression(assign_exp->left->right);
	}
	else if(assign_exp->left->type == "var"){
		return praser_var(assign_exp->left);
	}
	else if(assign_exp->left->type == "call"){
		return praser_call(assign_exp->left);
	}
	else if(assign_exp->left->type == "NUM"){
		string content = assign_exp->left->text;
		string tempname = "temp" + inttostr(tempNum);
		++tempNum;
		varNode newNode = createTempVar(tempname, "int");
		blockStack.back().varMap.insert({ tempname,newNode });
		addCode(tempname + " = "  + content);
		return newNode;
	}
}

//规则 27 28
varNode Praser::praser_call(struct AST* call_exp) {

		string funcName = call_exp->left->text;
		varNode newNode;
		cout<<"Debug:尝试调用函数："<<funcName<<endl;
		if (funcPool.find(funcName) == funcPool.end()) {
			error(call_exp->left->line, "Undefined function " + funcName);
		}

		//处理参数
		if (call_exp->left->right->right->left != NULL) {
			AST* argument_exp_list = call_exp->left->right->right->left;
			praser_argument_expression_list(argument_exp_list, funcName);
		}

		funcNode func = funcPool[funcName];
		
		//返回值
		if (func.rtype == "VOID") {
			addCode("CALL " + funcName);
		}
		else {
			string tempname = "temp" + inttostr(tempNum);
			++tempNum;

			newNode = createTempVar(tempname, funcPool[funcName].rtype);
			blockStack.back().varMap.insert({ tempname,newNode });
			addCode(tempname + " = CALL " + funcName);
		}

		return newNode;
}

//规则 29
void Praser::praser_argument_expression_list(struct AST* node, string funcName) {
	AST* argu_exp_list = node->left;
	funcNode func = funcPool[funcName];
	int i = 0;
	while (argu_exp_list->type == "arg-list") {
		//不断循环，直到arg-list指向的结点为expression
		varNode rnode = praser_expression(argu_exp_list->right->right);

		if(rnode.num<0){
			addCode("PARAM_PREP "+rnode.name);
		}else{
			addCode("PARAM_PREP var"+inttostr(rnode.num));
		}
		
		argu_exp_list = argu_exp_list->left;
		i++;
		if (func.paralist[func.paralist.size() - i].type != rnode.type) {
			error(argu_exp_list->line, "Wrong type arguments to function " + funcName);
		}
	}

	//最后一个参数
	varNode rnode = praser_expression(argu_exp_list);
	if(rnode.num<0){
			addCode("PARAM_PREP "+rnode.name);
		}else{
			addCode("PARAM_PREP var"+inttostr(rnode.num));
		}
	i++;
	if (func.paralist[func.paralist.size() - i].type != rnode.type) {
		error(argu_exp_list->line, "Wrong type arguments to function " + funcName);
	}
	if (i != func.paralist.size()) {
		error(argu_exp_list->line, "The number of arguments doesn't equal to the function parameters number.");
	}
}

//创建临时变量
struct varNode Praser::createTempVar(string name, string type) {
	varNode var;
	var.name = name;
	var.type = type;
	var.num = -1;	//以-1标记临时变量
	return var;
}

//根据变量名，返回一个varNode
struct varNode Praser::lookupNode(string name) {
	int N = blockStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (blockStack[i].varMap.find(name) != blockStack[i].varMap.end())
			return blockStack[i].varMap[name];
	}
	varNode temp;
	temp.num = -1;
	return temp;
}

//添加汇编代码
void Praser::addCode(string str)
{
	codeList.push_back(str);
}

//输出汇编代码
void Praser::print_code() {
	for (string s : codeList)
	{
		if (s.find("FUNC") != 0 && s.find("label") != 0)
		{
			cout << "\t";
		}
		cout << s << endl;
	}
}

//返回函数返回值类型
string Praser::getFuncRType() {
	int N = blockStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (blockStack[i].isfunc)
			return blockStack[i].func.rtype;
	}
	return "";
}

//返回行数
string Praser::getLabelName(){
	return "label_" + inttostr(labelNum++);
}

//获取结点名
string Praser::getNodeName(varNode node) {

	if (node.num < 0)
	{
		return node.name;
	}
	else
		return ("var" + inttostr(node.num));
}

//返回格式化表达式
string Praser::Gen_IR(string tempname, string op, varNode node1, varNode node2) {
	string result = tempname + " = ";

	if (node1.num < 0)
	{
		result += node1.name;
	}
	else
		result += "var" + inttostr(node1.num);

	result += " " + op + " ";	//空格和运算符

	if (node2.num < 0)
	{
		result += node2.name;
	}
	else
		result += "var" + inttostr(node2.num);
	return result;
}

//输出错误信息
void Praser::error(int line, string error) {
	cout << "line:" << line << "[Error] "<<error << endl;
	exit(1);
}
