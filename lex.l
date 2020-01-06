%option yylineno
%option noyywrap

%{
#include "AST.h"
#include "syntax.tab.h"
%}

%%


if	{yylval.node = new_node("IF",0,yylineno);   return(IF);}

else	{yylval.node = new_node("ELSE",0,yylineno); return(ELSE);}

int	{yylval.node = new_node("INT",0,yylineno); return(INT);}

return	{yylval.node = new_node("RETURN",0,yylineno); return(RETURN);}

void	{yylval.node = new_node("VOID",0,yylineno); return(VOID);}

while	{yylval.node = new_node("WHILE",0,yylineno); return(WHILE);}


"+"	{yylval.node = new_node("+",0,yylineno); return('+');}

"-"	{yylval.node = new_node("-",0,yylineno); return('-');}

"*"	{yylval.node = new_node("*",0,yylineno); return('*');}

"/"	{yylval.node = new_node("/",0,yylineno); return('/');}

"<"	{yylval.node = new_node("<",0,yylineno); return('<');}

"<="	{yylval.node = new_node("BE",0,yylineno); return(BE);}

">"	{yylval.node = new_node(">",0,yylineno); return('>');}

">="	{yylval.node = new_node(">=",0,yylineno); return(AE);}

"=="	{yylval.node = new_node("EQ",0,yylineno); return(EQ);}

"!="	{yylval.node = new_node("NE",0,yylineno); return(NE);}

"="	{yylval.node = new_node("=",0,yylineno); return('=');}

";"	{yylval.node = new_node(";",0,yylineno); return(';');}

","	{yylval.node = new_node(",",0,yylineno); return(',');}

"("	{yylval.node = new_node("(",0,yylineno); return('(');}

")"	{yylval.node = new_node(")",0,yylineno); return(')');}

"["	{yylval.node = new_node("[",0,yylineno); return('[');}

"]"	{yylval.node = new_node("]",0,yylineno); return(']');}

"{"	{yylval.node = new_node("{",0,yylineno); return('{');}

"}"	{yylval.node = new_node("}",0,yylineno); return('}');}


"//"[^\n]*    					    { }//忽略注释
"/*"(([^\*]*(\*[^\/])?)*)"*/"		{ }//忽略注释
[\n \t]	{ }	//忽略空白

[a-zA-Z]([a-zA-Z]|[0-9])*	{yylval.node = new_node("ID",0,yylineno); return(ID);}
[0-9]+		{yylval.node = new_node("NUM",0,yylineno); return(NUM);}

%%
