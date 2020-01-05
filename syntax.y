%{
#include <stdio.h>
#include "AST.h"
#include "Praser.h"

extern int yylineno;	//当前行号
extern FILE *yyin;		//读取文件指针
extern char *yytext;	//匹配内容
struct AST *root;		//语法树根节点

int yylex();
void yyerror(const char*);

%}

%union{
	struct AST* node;
}

%token	<node>	IF ELSE INT RETURN VOID WHILE
%token	<node>	'+' '-' '*' '/' '<' '>' '=' ';' ',' '(' ')' '[' ']' '{' '}'
%token	<node>	BE AE EQ NE
%token	<node>	ID NUM

%type	<node>	program declaration-list declaration var-declaration type-specifier fun-declaration
%type	<node>	params param-list param compound-stmt local-declarations statement-list statement
%type	<node>	expression-stmt selection-stmt iteration-stmt return-stmt expression
%type	<node>	var simple-expression relop additive-expression addop term mulop factor
%type	<node>	call args arg-list

%%

/*Rule 1*/
program:
	  declaration-list {	printf("Yes!\n");	root = new_node("program", 1, $1);	}
	;

/*Rule 2*/
declaration-list:
	  declaration-list declaration {	$$ = new_node("declaration-list", 2, $1, $2);	}
	| declaration {		$$ = new_node("declaration-list", 1, $1);	}
	;

/*Rule 3*/
declaration:
	  var-declaration {	$$ = new_node("declaration", 1, $1);	}
	| fun-declaration {	$$ = new_node("declaration", 1, $1);	}
	;

/*Rule 4*/
var-declaration:
	  type-specifier ID ';' {	$$ = new_node("var-declaration", 3, $1, $2, $3);	}
	| type-specifier ID '[' NUM ']' ';' {	$$ = new_node("var-declaration", 6, $1, $2, $3, $4, $5, $6);	}
	;

/*Rule 5*/
type-specifier:
	  INT 	{	$$ = new_node("type-specifier", 1, $1);	}
	| VOID	{	$$ = new_node("type-specifier", 1, $1);	}
	;

/*Rule 6*/
fun-declaration:
	  type-specifier ID '(' params ')' compound-stmt {	$$ = new_node("fun-declaration", 6, $1, $2, $3, $4, $5, $6);	}
	;

/*Rule 7*/
params:
      param-list {	$$ = new_node("params", 1, $1);	}
	| VOID	{	$$ = new_node("params", 1, $1);	}
	;

/*Rule 8*/
param-list:
      param-list ',' param {	$$ = new_node("param-list", 3, $1, $2, $3);	}
	| param {	$$ = new_node("param-list", 1, $1);	}
	;

/*Rule 9*/
param:
      type-specifier ID {	$$ = new_node("param", 2, $1, $2);	}
	| type-specifier ID '[' ']' {	$$ = new_node("param", 4, $1, $2, $3, $4);	}
	;

/*Rule 10*/
compound-stmt:
	  '{' local-declarations statement-list '}' {	$$ = new_node("compound-stmt", 4, $1, $2, $3, $4);	}
	;

/*Rule 11*/
local-declarations:
	  {	$$ = new_node("local-declarations", -1);	/*empty*/	}
	| local-declarations var-declaration {	$$ = new_node("local-declarations", 2, $1, $2);	}
	;

/*Rule 12*/
statement-list:
	  {	$$ = new_node("statement-list", -1);	/*empty*/	}
	| statement-list statement {	$$ = new_node("statement-list", 2, $1, $2);	}
	;

/*Rule 13*/
statement:
	  expression-stmt  {	$$ = new_node("statement", 1, $1);	}
	| compound-stmt	   {	$$ = new_node("statement", 1, $1);	}
	| selection-stmt   {	$$ = new_node("statement", 1, $1);	}
	| iteration-stmt   {	$$ = new_node("statement", 1, $1);	}
	| return-stmt	   {	$$ = new_node("statement", 1, $1);	}
	;

/*Rule 14*/
expression-stmt:
	  expression ';' {	$$ = new_node("expression-stmt", 2, $1, $2);	}
	| ';'	{	$$ = new_node("expression-stmt", 1, $1);	}
	;

/*Rule 15*/
selection-stmt:
	  IF '(' expression ')' statement {	$$ = new_node("selection-stmt", 5, $1, $2, $3, $4, $5);	}
	| IF '(' expression ')' statement ELSE statement {	$$ = new_node("selection-stmt", 7, $1, $2, $3, $4, $5, $6, $7);	}
	;

/*Rule 16*/
iteration-stmt:
	  WHILE '(' expression ')' statement {	$$ = new_node("iteration-stmt", 5, $1, $2, $3, $4, $5);	}
	;

/*Rule 17*/
return-stmt:
	  RETURN ';'	{	$$ = new_node("return-stmt", 2, $1, $2);	}
	| RETURN expression ';'	{	$$ = new_node("return-stmt", 3, $1, $2, $3);	}
	;

/*Rule 18*/
expression:
	  var '=' expression {	$$ = new_node("expression", 3, $1, $2, $3);	}
	| simple-expression  {	$$ = new_node("expression", 1, $1);	}
	;

/*Rule 19*/
var:
   	  ID	{	$$ = new_node("var", 1, $1);	}
	| ID '[' expression ']' {	$$ = new_node("var", 4, $1, $2, $3, $4);	}
	;

/*Rule 20*/
simple-expression:
	  additive-expression relop additive-expression {	$$ = new_node("simple-expression", 3, $1, $2, $3);	}
	| additive-expression {	$$ = new_node("simple-expression", 1, $1);	}
	;

/*Rule 21*/
relop:
       BE  {	$$ = new_node("relop", 1, $1);	}
	| '<'  {	$$ = new_node("relop", 1, $1);	}
	| '>'  {	$$ = new_node("relop", 1, $1);	}
	|  AE  {	$$ = new_node("relop", 1, $1);	}
	|  EQ  {	$$ = new_node("relop", 1, $1);	}
	|  NE  {	$$ = new_node("relop", 1, $1);	}
	;

/*Rule 22*/
additive-expression:
	  additive-expression addop term  {	$$ = new_node("additive-expression", 3, $1, $2, $3);	}
	| term  {	$$ = new_node("additive-expression", 1, $1);	}
	;

/*Rule 23*/
addop:
      '+' {	$$ = new_node("addop", 1, $1);	}
	| '-' {	$$ = new_node("addop", 1, $1);	}
	;
	
/*Rule 24*/
term:
      term mulop factor {	$$ = new_node("term", 3, $1, $2, $3);	}
	| factor {	$$ = new_node("term", 1, $1);	}
	;

/*Rule 25*/
mulop:
      '*' {	$$ = new_node("mulop", 1, $1);	}
	| '/' {	$$ = new_node("mulop", 1, $1);	}
	;

/*Rule 26*/
factor:
      '(' expression ')' {	$$ = new_node("factor", 3, $1, $2, $3);	}
	| var 	{	$$ = new_node("factor", 1, $1);	}
	| call	{	$$ = new_node("factor", 1, $1);	}
	| NUM	{	$$ = new_node("factor", 1, $1);	}
	;

/*Rule 27*/
call:
      ID '(' args ')' {	$$ = new_node("call", 4, $1, $2, $3, $4);	}
	;

/*Rule 28*/
args:
	  {	$$ = new_node("args", -1);	/*empty*/	}
    | arg-list	{	$$ = new_node("args", 1, $1);	}
	;

/*Rule 29*/
arg-list:
	  arg-list ',' expression {	$$ = new_node("arg-list", 3, $1, $2, $3);	}
	| expression	{	$$ = new_node("arg-list", 1, $1);	}
	;

%%
void yyerror(char const*s)
{
	printf("error in line %d: %s\n", yylineno, s);
}

int main(int argc, char *argv[])
{
	yyin = stdin;
	if(argc > 1)
	{
		if(!(yyin = fopen(argv[1], "r")))
		{
			printf("Cannot open this file %s\n", argv[1]);
			return (1);
		}
	}

	yyparse();
	printf("---------------------------lex_end----------------------------\n");

	print_AST(root,0);
	printf("-----------------------print_AST_end--------------------------\n");

//	Praser praser(root);
	fclose(yyin);
	return 0;
}
