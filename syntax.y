%{
#include <stdio.h>
extern int yylineno;	//行数
extern FILE *yyin;	//读取文件

int yylex();
void yyerror(const char*);

%}

%token	IF ELSE INT RETURN VOID WHILE
%token	'+' '-' '*' '/' '<' '>' '=' ';' ',' '(' ')' '[' ']' '{' '}'
%token  BE AE EQ NE
%token	ID NUM

%%

/*Rule 1*/
program:	
	  declaration-list {printf("Yes!\n");}
	;

/*Rule 2*/
declaration-list:
	  declaration-list declaration {}
	| declaration {}
	;

/*Rule 3*/
declaration:
	  var-declaration {}
	| fun-declaration {}
	;

/*Rule 4*/
var-declaration:
	  type-specifier ID ';' {}
	| type-specifier ID '[' NUM ']' ';' {}
	;

/*Rule 5*/
type-specifier:
	  INT 	{}
	| VOID	{}
	;

/*Rule 6*/
fun-declaration:
	  type-specifier ID '(' params ')' {}
	| compound-stmt {}
	;

/*Rule 7*/
params:
      	  param-list {}
	| VOID	{}
	;

/*Rule 8*/
param-list:
      	  param-list ',' param {}
	| param {}
	;

/*Rule 9*/
param:
     	  type-specifier ID {}
	| type-specifier ID '[' ']' {}
	;

/*Rule 10*/
compound-stmt:
	  '{' local-declarations statement-list '}' {}
	;

/*Rule 11*/
local-declarations:
	  {}	/*empty*/
	| local-declarations var-declaration {}
	;

/*Rule 12*/
statement-list:
	  {}	/*empty*/
	| statement-list statement {}
	;

/*Rule 13*/
statement:
	  expression-stmt  {}
	| compound-stmt	   {}
	| selection-stmt   {}
	| iteration-stmt   {}
	| return-stmt	   {}
	;

/*Rule 14*/
expression-stmt:
	  expression ';' {}
	| ';'	{}
	;

/*Rule 15*/
selection-stmt:
	  IF '(' expression ')' statement {}
	| IF '(' expression ')' statement ELSE statement {}
	;

/*Rule 16*/
iteration-stmt:
	  WHILE '(' expression ')' statement {}
	;

/*Rule 17*/
return-stmt:
	  RETURN ';'
	| RETURN expression ';'
	;

/*Rule 18*/
expression:
	  var '=' expression {}
	| simple-expression  {}
	;

/*Rule 19*/
var:
   	  ID {}
	| ID '[' expression ']' {}
	;

/*Rule 20*/
simple-expression:
	  additive-expression relop additive-expression {}
	| additive-expression {}
	;

/*Rule 21*/
relop:
     	   BE  {}
	| '<'  {}
	| '>'  {}
	|  AE  {}
	|  EQ  {}
	|  NE  {}
	;

/*Rule 22*/
additive-expression:
	  additive-expression addop term  {}
	| term  {}
	;

/*Rule 23*/
addop:
     	  '+' {}
	| '-' {}
	;
	
/*Rule 24*/
term:
    	  term mulop factor {}
	| factor {}
	;

/*Rule 25*/
mulop:
     	  '*' {}
	| '/' {}
	;

/*Rule 26*/
factor:
      	  '(' expression ')' {}
	| var 	{}
	| call	{}
	| NUM	{}
	;

/*Rule 27*/
call:
    	  ID '(' args ')' {}
	;

/*Rule 28*/
args:
    	  {}	/*empty*/
    	| arg-list
	;

/*Rule 29*/
arg-list:
	  arg-list ',' expression {}
	| expression
	;

%%
void yyerror(char const*s)
{
	printf("error in line %d: %s\n", yylineno, s);
}

int main(argc, argv)
int argc;
char **argv;
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
	fclose(yyin);
	return 0;
}
