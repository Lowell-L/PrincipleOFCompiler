%option yylineno
%option noyywrap

%{
#include "syntax.tab.h"
%}

%%


if	{return(IF);}

else	{return(ELSE);}

int	{return(INT);}

return	{return(RETURN);}

void	{return(VOID);}

while	{return(WHILE);}


"+"	{return('+');}

"-"	{return('-');}

"*"	{return('*');}

"/"	{return('/');}

"<"	{return('<');}

"<="	{return(BE);}

">"	{return('>');}

">="	{return(AE);}

"=="	{return(EQ);}

"!="	{return(NE);}

"="	{return('=');}

";"	{return(';');}

","	{return(',');}

"("	{return('(');}

")"	{return(')');}

"["	{return('[');}

"]"	{return(']');}

"{"	{return('{');}

"}"	{return('}');}


"//"[^\n]*    					    { }//忽略注释
"/*"(([^\*]*(\*[^\/])?)*)"*/"		{ }//忽略注释
[\n \t]	{ }	//忽略空白

[a-zA-Z]+	{return(ID);}
[0-9]+		{return(NUM);}

%%
