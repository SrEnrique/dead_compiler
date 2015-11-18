%{
#include <string>
#include <iostream>
#include <cstring>
#include "node.h"
#include "parser.hpp"

#define SAVE_TOKEN  yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t)    (yylval.token = t)
	bool debug = 1;
	int line = 1;
	int col = 0;
%}

%option noyywrap

%%

[ \t]					        {col++;}
[\n]							{	line++;
									col = 0;
								}
"externo"                      {return TOKEN(TEXTERN);}
"regresa"				        {return TOKEN(TRETURN);}
"variable"						{
									if (debug)
										printf("-->Enconte la var \n"); 
									
									return TOKEN(TVAR);
								}
"funcion"						{
									if (debug)
										printf("-->Enconte la var \n"); 
									
									return TOKEN(TFUN);
								}


":"                             {
									if (debug)
									 	printf("-->Enconte los : \n");

									return TOKEN(TDOSPUNTOS);
								}

"printer"						{
									return TOKEN(TPRINT);		
								}

"inicio"         					return TOKEN(TLBRACE);
"fin"					          	return TOKEN(TRBRACE);

"si"							{
									return TOKEN(TIF);
								}
"sino"							{ return TOKEN(TELSE);} 

\".*\"                  		{//"
									

									return TOKEN(TSTR); 
								}

[a-zA-Z_][a-zA-Z0-9_]* 			{
									SAVE_TOKEN; 
									return TIDENTIFIER;
								}

\/\/.*?\n 						{/*comentarios*/}
"/*"[^"*/"]*"*/"				{/*Mas comentarios*/}								

[0-9]+\.[0-9]* 			    	SAVE_TOKEN; return TDOUBLE;
[0-9]+					        SAVE_TOKEN; return TINTEGER;

":"								{return TOKEN(TDOSPUNTOS);}
"="						        return TOKEN(TEQUAL);
"=="				          	return TOKEN(TCEQ);
"!="			          		return TOKEN(TCNE);
"<"				          		return TOKEN(TCLT);
"<="	          				return TOKEN(TCLE);
">"				          		return TOKEN(TCGT);
">="					        return TOKEN(TCGE);

"("	          					return TOKEN(TLPAREN);
")"					          	return TOKEN(TRPAREN);


"."         					return TOKEN(TDOT);
","				          		return TOKEN(TCOMMA);

"+"				          		return TOKEN(TPLUS);
"-"		          				return TOKEN(TMINUS);
"*"		          				return TOKEN(TMUL);
"/"				          		return TOKEN(TDIV);

.                       printf("error token en la linea %i: %i\n", line, col); yyterminate();

%%
