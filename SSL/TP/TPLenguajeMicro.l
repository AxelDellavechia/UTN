%{

	/********************
	****Definiciones****
	********************/

	#include "TPLenguajeMicro.tab.h"
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
%}

	/********************
	**Reglas y Patrones**
	********************/

%option noyywrap
%option yylineno

espacio			[ \t]+
identificador	[a-zA-Z][a-zA-Z0-9]{0,31}
constante		[0-9][0-9]*

%%

	/********************
	**Código de Usuario**
	********************/

\n 														{ yylineno++; }
{espacio}												{/*Ignorar esopacio*/};

"inicio"												return(INICIO);
"fin"													return(FIN);
"escribir" 												return(ESCRIBIR);
"leer" 													return(LEER);
	
{identificador} 										{
															strcpy(yylval.identificador,yytext); 
															return IDENTIFICADOR ;
														};
{constante}												{
															yylval.constante=atoi(yytext);
															return CONSTANTE;
														};
														
"-"														return(SIMB_MENOS);
":="													return(SIMB_ASIG);
"+"														return(SIMB_MAS);
";"														return(SIMB_PYC);
","														return(SIMB_COMA);
"("														return(PARENT_IZQUIERDO);
")"														return(PARENT_DERECHO);
.														{printf("\nLa palabra leida: %s en la linea %d,no partece al lenguaje por lo cual se produjo un error lexicografico\n",yytext,yylineno);};

%%
