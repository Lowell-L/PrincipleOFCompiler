#include "AST.h"
using namespace std;

string non_terminal[]={ //非终结符
    "Program",        
    "declaration_list",
    "declaration" ,
    "var_declaration",
    "type_specifier",
    "fun_declaration",
    "params",
    "param_list",
    "param",
    "compound_stmt",
    "local_declarations",
    "statement_list",
    "statement",
    "expression_stmt",
    "selection_stmt",
    "iteration_stmt",
    "return_stmt",
    "expression",
    "var",
    "simple_expression",
    "relop",
    "additive_expression",
    "addop",
    "term",
    "mulop",
    "factor",
    "call",
    "args",
    "arg_list",
};

int non_terminal_len=sizeof(non_terminal)/sizeof(non_terminal[0]);
int printtreestack[1000];

//构建语法树
struct AST* new_node(string type, int num,...) { 

    if(num==0){         //token
        cout<<"get a token: " <<type <<endl;
    }

    va_list valist;//接收不定量参数 char* 指向当前参数的指针

    struct AST* head = new AST();
    struct AST* temp = NULL;
    head->left = NULL;
    head->right = NULL;
    head->text = "";
    head->type = type;
    va_start(valist,num);       //使va_list指向起始的参数
    if(num > 0) {               //不是叶子节点
        temp = va_arg(valist,struct AST*);  //检索参数,第二个参数为数据类型，返回参数
        head->left = temp;      //左孩子，
        head->line = temp->line;
        if(num == 1) {          //只有一个参数，直接赋值
            head->text = temp->text;
        }
        else {                  //有多个参数，用右兄弟表示
            for(int i = 1; i < num; ++i ) {
                temp->right = va_arg(valist,struct AST*);
                temp = temp->right;
            }
        }
    }
    else if(num == 0){                      //叶结点                  
        int line = va_arg(valist,int);
        head->line = line;
        if(head->type == "NUM") {  //整型常量
            int value = atoi(yytext);       //字符串转化为整型
            head->text = inttostr(value);   //整型转换为字符串 标准输出
        }
        else { //直接赋值
            head->text = yytext;
        }
    }
    else {                      //空串
        cout<<"pi pei kong chuan" <<endl;
    }
    return head;
}


//打印抽象语法树
void print_AST(AST *head,int leavel) {
    if(head != NULL) {
        string type = head->type;

        //嵌套层次
        for(int i = 0; i < leavel; ++i) {
            if(printtreestack[i] == 0){
                cout<<"  ";
            }else{
                cout<<"| ";
            }
        }
        cout<<"+-" << head->type;

        //有真实值的类型 
        if(head->type == "ID" || head->type == "NUM") {
            cout << " :" << head->text;
        }
        
        cout << endl;

        if(head->right != NULL){
            printtreestack[leavel]=1;
        }

        print_AST(head->left,leavel+1);
        printtreestack[leavel]=0;   //将高位的1归零
        print_AST(head->right,leavel);
    }
}


//真值返回 1
int is_non_terminal(string str){
    int i=0;
    int is_non_terminal=0;
    for(i=0;i<non_terminal_len;i++){
        if(non_terminal[i]==str){
            is_non_terminal=1;
            break;
        }
    }
    return is_non_terminal;
}

//整型转字符串
string inttostr(int n) {
	char buf[10];
	sprintf(buf, "%d", n);
	string b = buf;
	return b;
}

//字符串转整型
int strtoint(string s) {
	int n;
	n = atoi(s.c_str());
	return n;
}
