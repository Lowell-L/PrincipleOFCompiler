#include "Praser.h"

using namespace std;

Praser::Praser(AST* root) {//���캯��
	this->root = root;
	praserInit();
	printf("---------------------debug_info_end---------------------------\n");
	print_code();
	printf("---------------------inner_code_end---------------------------\n");
	//asmcode.CodeGen(codeList);
	//asmcode.printCode();
	printf("--------------------objcoed_gen_end---------------------------\n");
}

void Praser::praserInit() { //��ʼ��
	Block wholeBlock; 		//������������Ĵ������Ϣ��ȫ�֣�
	blockStack.push_back(wholeBlock);  
	
	//�������ú��� input������һ��int ������  output����ӡһ��intֵ
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

	praserAST(root);		//��ʼ�����﷨��

	blockStack.pop_back();
}

void Praser::praserAST(struct AST* node) {
	if (node == NULL)
		return;
	else if (node->type == "fun-declaration") { //��������
		praser_fun_declaration(node);
		return;
	}
	else if (node->type == "var-declaration") {	//��������
		praser_var_declaration(node);
		return;
	}
	else if (node->type == "statement") {		//�������
		praser_statement(node);
		return;
	}  
	else{	//�������µݹ����
		praserAST(node->left);
		praserAST(node->right);
	}
}

//���� 6��7	������������
void Praser::praser_fun_declaration(struct AST* node) {
	AST* type_specifier;			//�������ͽ��
	type_specifier = node->left;
	
	AST* ID = node->left->right;	//�������ƽ��

	AST* compound_stmt = ID->right->right->right->right;
	
	string funcType = type_specifier->left->text;	//����ֵ����

	string funcName = ID->text;				//������
	
	funcNode declarFunc;					//�������

	if (funcPool.find(funcName) != funcPool.end()) {
		//�ú�������funcPool���ҵ��������ظ�����
		error(ID->line, "�����ظ����壺" + funcName );
	}

	Block funBlock;
	funBlock.isfunc = true;
	funBlock.func.name = funcName;
	funBlock.func.rtype = funcType;
	//��������¼�ڿ��ڲ���ӵ�������
	blockStack.push_back(funBlock);
	funcPool.insert({funcName,funBlock.func});

	cout<<"Debug:�ɹ�����Block�Ͳ�����funcPool,������"<<funcName<<endl;

	//�м����
	addCode("FUNC " + funcName + ":");

	//��ȡ�����β��б�
	if(ID->right->right->left->type == "param-list"){
		cout<<"Debug:��ʼ����һ����������param-list���� function"<<endl;
		praser_param_list(ID->right->right->left, funcName);
		cout<<"Debug:�����������"<<endl;
	}
	else
	{
		//�����б� VOID ������������б�
	}
	
	//��ʱ�������е�func�Ѿ�����˲����б�
	funcNode func = funcPool[funcName];

	//����Block��func�Ĳ����б�
	funBlock.func = func;
	//��������������
	praser_compound_stmt(compound_stmt->left->right);

	//���������󣬵�����Ӧ��block
	blockStack.pop_back();

}

//���� 8	���������
void Praser::praser_param_list(struct AST* node,string funcName) {
	if (node->left->type == "param-list") {
		praser_param_list(node->left, funcName);
		praser_param(node->left->right->right, funcName);
	}
	else if (node->left->type == "param") {
		praser_param(node->left,funcName);
	}
}

//���� 9	��ȡ�����β�����
void Praser::praser_param(struct AST* node, string funcName) {

	AST* type_specifier = node->left;	//�������ͽ��
	AST* ID = node->left->right;		//�������ƽ��

	string typeName = type_specifier->left->text;		//��������
	string varName = ID->text;		//��������

	varNode newnode;
	newnode.name = varName;
	newnode.type = typeName;
	newnode.num = varNum++;			

	blockStack.back().func.paralist.push_back(newnode);	//�������������еĺ�������β��б���
	cout<<"Debug:�ɹ�������:"<<varName<<"��ӵ�:"<<funcName<<endl;

	funcPool[funcName].paralist.push_back(newnode);		//�������뺯�����еĺ�������β��б���
	
	blockStack.back().varMap.insert({varName,newnode});	//���������β���ӵ���ǰ��ı�������
	addCode("PARAM_IN var" + inttostr(newnode.num));	
}

//����10	��������
void Praser::praser_compound_stmt(struct AST* node) {
	//������������compound_stmt
	//����{}���еĴ��룬����һ���µĿ飬�µ������ռ�
	Block newblock;
	blockStack.push_back(newblock);
	
	praserAST(node);  			//local_declarations
	//praserAST(node->right);		//statement_list

	blockStack.pop_back();
}

//���� 4	������������
void Praser::praser_var_declaration(struct AST *node) {
	
	cout<<"Debug:��ʼ���� praser_var_declaration"<<endl;

	AST* type_specifier = node->left;	//�������ͽ��
	AST* ID = node->left->right;		//�������ƽ��

	string typeName = type_specifier->left->text;		//��������
	string varName = ID->text;		//��������
			
	if ( blockStack.back().varMap.find(varName) == blockStack.back().varMap.end() ) {
		varNode newnode;
		newnode.name = varName;
		newnode.type = typeName;
		newnode.num = varNum++;
		blockStack.back().varMap.insert({varName, newnode});	//�����������ر����б���
	}
	else{
		error(node->left->right->line, "�����ظ�����");
	}

	return ;
}

//����13	����statement���
void Praser::praser_statement(struct AST* node) {
	struct AST* next = node->left;
	if(node->left->type == "expression-stmt") {//����;���� expression ';'
		praser_expression_stmt(node->left);
	}
	else if (node->left->type == "compound-stmt") {//�������  {������} 
		praser_compound_stmt(node->left);
	}
	else if (node->left->type == "selection-stmt") { // if���
		praser_selection_stmt(node->left);
	}
	else if (node->left->type == "iteration-stmt") {//������� while
		praser_iteration_stmt(node->left);
	}
	else if (node->left->type == "return-stmt") { //return
		praser_return_stmt(node->left);
	}
}

//���� 14	���ʽ���
void Praser::praser_expression_stmt(struct AST *node) {
	if (node->left->type == "expression") {
		praser_expression(node->left);
	}
	else if(node->left->type == ";"){
		//����� ����;
	}
}	

//���� 18	����expression
varNode Praser::praser_expression(struct AST* node) {
	if (node->left->type == "var") {
		return praser_assignment_expression(node->left);
	}
	else if(node->left->type == "simple_expression") {
		return praser_simple_expression(node->left);
	}
}

//���� 15	����if���
void Praser::praser_selection_stmt(struct AST* node) {
	cout<<"Debug:142 ��ʼ����IF��䣺"<<node->left->type<<endl;
	if (node->left->type == "if") {
		if (node->left->right->right->right->right->right == NULL) {
			//���һ���µ�block  û��else���
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

			//������ӵ�block
			blockStack.pop_back();

		}
		else if (node->left->right->right->right->right->right->type == "else") {
			//���һ���µ�block
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
			//������ӵ�block
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
		cout<<"Debug:dsa praser_selection_stmt,δ�������䣺"<<node->left->type<<endl;
	}
}

//���� 16	����while���
void Praser::praser_iteration_stmt(struct AST* node) {
	if (node->left->type == "While") {

		//���һ���µ�block
		Block newblock;
		blockStack.push_back(newblock);

		struct AST* expression = node->left->right->right;
		struct AST* statement = node->left->right->right->right->right;

		string label1 = getLabelName(); //�ж����Ŀ�ʼ
		string label2 = getLabelName(); //ѭ�������
		string label3 = getLabelName(); //ѭ�������

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
		
		//������ӵ�block
		blockStack.pop_back();
	}
	else{
		cout<<"Debug:dsa praser_iteration_stmt,δ�������䣺"<<node->left->type<<endl;
	}
}

//���� 17	����return���
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
				cout <<"Dubug:123 �������Ͳ�ƥ��"<< rnode.type <<"and"<<funcType <<endl;
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
			cout<<"Dubug:dqw δ֪�Ĵ���"<<endl;
		}
	}
}

//���� 18.1	��ֵ������
varNode Praser::praser_assignment_expression(struct AST* assign_exp) {	//���ر����ڵ�
	cout<<"Debug:��ʼ������ֵ���:"<<assign_exp->type<<endl;
	
	struct AST* primary_exp=assign_exp;
	string op = primary_exp->right->type;
	struct AST* next_assign_exp = primary_exp->right->right;

	varNode node1 = praser_var(primary_exp);
	varNode node2 = praser_expression(next_assign_exp);
	varNode node3;

	if (op == "=") {
		node3 = node2;
	}

	if( node3.num < 0){	//����ʱ���� �� a=1+1;
		addCode("var"+ inttostr(node1.num) +" = "+ node3.name);
	}
	else{				//����֪���� �� a=b;
		addCode("var"+ inttostr(node1.num) +" = var"+inttostr(node3.num));
	}		

	return node1;		
}

//���� 19
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

//���� 20
varNode Praser::praser_simple_expression(struct AST* assign_exp){
	cout<<"Debug:czxc ��ʼ�������ʽ:"<<assign_exp->type<<endl;
	
	if(assign_exp->left->right == NULL)
		return praser_additive_expression(assign_exp->left);
	else{
		varNode additive_expression_1 = praser_additive_expression(assign_exp->left);
		varNode additive_expression_2 = praser_additive_expression(assign_exp->left->right->right);
		return praser_relop(additive_expression_1, assign_exp->left->right, additive_expression_2);
	}
}

//���� 21	
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
			error(relop->line, "���Ͳ�ƥ�䣡");
		}

		string tempname = "temp" + inttostr(tempNum);
		++tempNum;
		varNode newnode = createTempVar(tempname, "int");
		blockStack.back().varMap.insert({ tempname,newnode});
		addCode(Gen_IR(tempname, op, additive_expression_1, additive_expression_1));

		return newnode;
	}
}

//���� 22	
varNode Praser::praser_additive_expression(struct AST* assign_exp){
	if(assign_exp->left->right == NULL)
		return praser_term(assign_exp->left);
	else{
		varNode additive_expression = praser_additive_expression(assign_exp->left);
		varNode term = praser_term(assign_exp->left->right->right);
		return praser_addop(additive_expression, assign_exp->left->right, term);
	}
}

//���� 23
varNode Praser::praser_addop(varNode additive_expression, AST* addop, varNode term){
	if(addop->type == "+" || addop->type == "-"){
		if (additive_expression.type != term.type) {
			cout<<"Debug:��ͬ�Ĳ������ͣ�+����"<< additive_expression.type << "��" <<term.type<<endl;
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

//���� 24
varNode Praser::praser_term(struct AST* assign_exp){
	if(assign_exp->left->right == NULL)
		return praser_factor(assign_exp->left);
	else{
		varNode term = praser_term(assign_exp->left);
		varNode factor = praser_factor(assign_exp->left->right->right);
		return praser_mulop(term, assign_exp->left->right, factor);
	}
}

//���� 25
varNode Praser::praser_mulop(varNode term, AST* mulop, varNode factor){
	if(mulop->type == "*" || mulop->type == "/"){
		if (term.type != factor.type) {
			cout<<"Debug:��ͬ�Ĳ������ͣ�+����"<< term.type << "��" <<factor.type<<endl;
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

//���� 26
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

//���� 27 28
varNode Praser::praser_call(struct AST* call_exp) {

		string funcName = call_exp->left->text;
		varNode newNode;
		cout<<"Debug:���Ե��ú�����"<<funcName<<endl;
		if (funcPool.find(funcName) == funcPool.end()) {
			error(call_exp->left->line, "Undefined function " + funcName);
		}

		//�������
		if (call_exp->left->right->right->left != NULL) {
			AST* argument_exp_list = call_exp->left->right->right->left;
			praser_argument_expression_list(argument_exp_list, funcName);
		}

		funcNode func = funcPool[funcName];
		
		//����ֵ
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

//���� 29
void Praser::praser_argument_expression_list(struct AST* node, string funcName) {
	AST* argu_exp_list = node->left;
	funcNode func = funcPool[funcName];
	int i = 0;
	while (argu_exp_list->type == "arg-list") {
		//����ѭ����ֱ��arg-listָ��Ľ��Ϊexpression
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

	//���һ������
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

//������ʱ����
struct varNode Praser::createTempVar(string name, string type) {
	varNode var;
	var.name = name;
	var.type = type;
	var.num = -1;	//��-1�����ʱ����
	return var;
}

//���ݱ�����������һ��varNode
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

//��ӻ�����
void Praser::addCode(string str)
{
	codeList.push_back(str);
}

//���������
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

//���غ�������ֵ����
string Praser::getFuncRType() {
	int N = blockStack.size();
	for (int i = N - 1; i >= 0; i--) {
		if (blockStack[i].isfunc)
			return blockStack[i].func.rtype;
	}
	return "";
}

//��������
string Praser::getLabelName(){
	return "label_" + inttostr(labelNum++);
}

//��ȡ�����
string Praser::getNodeName(varNode node) {

	if (node.num < 0)
	{
		return node.name;
	}
	else
		return ("var" + inttostr(node.num));
}

//���ظ�ʽ�����ʽ
string Praser::Gen_IR(string tempname, string op, varNode node1, varNode node2) {
	string result = tempname + " = ";

	if (node1.num < 0)
	{
		result += node1.name;
	}
	else
		result += "var" + inttostr(node1.num);

	result += " " + op + " ";	//�ո�������

	if (node2.num < 0)
	{
		result += node2.name;
	}
	else
		result += "var" + inttostr(node2.num);
	return result;
}

//���������Ϣ
void Praser::error(int line, string error) {
	cout << "line:" << line << "[Error] "<<error << endl;
	exit(1);
}
