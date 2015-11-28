%{
	#include "node.h"
    #include <cstdio>
    #include <cstdlib>
    using namespace std;
	NBlock *programBlock; /* the top level root node of our final AST */

	extern int yylex();
	extern int line;
	extern char* yytext;
	extern bool debug;
	void yyerror(const char *s) { std::printf("Error: %s\nLinea: %i\nno esperaba %s\n", s, line ,yytext);std::exit(1); }
%}

/* Represents the many different ways we can access our data */
%union {
	Node *node;
	NBlock *block;
	NExpression *expr;
	NStatement *stmt;
	NIdentifier *ident;
	NVariableDeclaration *var_decl;
	NVariableDeclaration *var_decl_f;
	std::vector<NVariableDeclaration*> *varvec;
	std::vector<NExpression*> *exprvec;
	std::string *string;
	int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l flex file. We also define the node type
   they represent.
 */
%token <string> TIDENTIFIER TINTEGER TDOUBLE TSTR


%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL  
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT
%token <token> TPLUS TMINUS TMUL TDIV
%token <token> TRETURN TEXTERN TVAR TDOSPUNTOS TPRINT TFUN  TIF TELSE TFOR TWIL TCASOS TCASO
/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <ident> ident 
%type <expr> numeric expr cadena
%type <varvec> func_decl_args
%type <exprvec> call_args
%type <block> program stmts block
%type <stmt> stmt var_decl func_decl extern_decl if_dec if_dec2 for_dec wil_dec block_case dec_case
%type <token> comparison 

/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV

%start program

%%

program : stmts { programBlock = $1; }

		;
		
stmts : stmt { $$ = new NBlock(); $$->statements.push_back($<stmt>1); }
	  | stmts stmt { $1->statements.push_back($<stmt>2); }
	  ;

stmt : TVAR var_decl {$$ = $2;}
	 | TFUN func_decl {$$ = $2;}
	 | if_dec {$$ = $1;}
	 | if_dec2 {$$ = $1;}
	 | for_dec {$$ = $1;}
	 | dec_case {$$ = $1;}
	 | wil_dec {$$= $1;}
	 | var_decl | func_decl | extern_decl
	 | expr { $$ = new NExpressionStatement(*$1); }
	 | TRETURN expr { $$ = new NReturnStatement(*$2); }
     ;




block : TLBRACE stmts TRBRACE { $$ = $2; }
	  | TLBRACE TRBRACE { $$ = new NBlock(); }
	  ;


var_decl :  ident TDOSPUNTOS ident { $$ = new NVariableDeclaration(*$3, *$1); }
		 |  ident TEQUAL expr TDOSPUNTOS ident { $$ = new NVariableDeclaration(*$5, *$1, $3); }
		 ;


extern_decl : TEXTERN ident ident TLPAREN func_decl_args TRPAREN
                { $$ = new NExternDeclaration(*$2, *$3, *$5); delete $5; }
            ;

func_decl :  ident TLPAREN func_decl_args TRPAREN TDOSPUNTOS ident block 
			{
				//Funcion declaracion :D 
				//me tipo, nombre, args, block instru y delete bloc
				$$ = new NFunctionDeclaration(*$6, *$1, *$3, *$7); delete $3; 
			}
		  ;

if_dec 	: TIF TLPAREN expr TRPAREN block { 
				/*$$ = new BranchStatement( $3, *$5 );*/ 
				if (debug)
					printf(";------>Encontre los if - pero no se hacer nada\n");
				$$ = new BranchStatement( $3, *$5 );
			}
		;
if_dec2 : TIF TLPAREN expr TRPAREN block TELSE block { 
				if (debug)
					printf(";----->Encontre los el else - pero no se hacer nada\n");
				$$ = new BranchStatement( $3, *$5, *$7 );}
		;

	
func_decl_args : /*blank*/  { $$ = new VariableList(); }
		  | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
		  | func_decl_args TCOMMA var_decl { $1->push_back($<var_decl>3); }
		  ;

ident : TIDENTIFIER { 
						
						$$ = new NIdentifier(*$1); delete $1; 
					}
	  ;

numeric : TINTEGER { $$ = new NInteger(atol($1->c_str())); delete $1; }
		| TDOUBLE { $$ = new NDouble(atof($1->c_str())); delete $1; }
		;





expr : ident TEQUAL expr { $$ = new NAssignment(*$<ident>1, *$3); }
	 | ident TLPAREN call_args TRPAREN { $$ = new NMethodCall(*$1, *$3); delete $3; }
	 | ident { $<ident>$ = $1; }
	 | numeric
	 | cadena
     | expr TMUL expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | expr TDIV expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | expr TPLUS expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | expr TMINUS expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
 	 | expr comparison expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | TLPAREN expr TRPAREN { $$ = $2; }
	 ;
cadena 	: TSTR 	{ 
					printf(";%s\n", yytext );
					$$ = new NString(yytext); delete $1; 
				}
		;
	
call_args : /*blank*/  { $$ = new ExpressionList(); }
		  | expr { $$ = new ExpressionList(); $$->push_back($1); }
		  | call_args TCOMMA expr  { $1->push_back($3); }
		  ;
//		   1  |   2   |  3  |  4   | 5  |  6   | 6  |  7    | 8
for_dec : TFOR TLPAREN expr TCOMMA expr TCOMMA expr TRPAREN block
			{ 
				if(debug) printf(";--------> :D \n");
				//inicio, end paso block
				$$ = new ForStatement( $3, $5, $7, *$9 ); 

			}
		;

wil_dec : TWIL TLPAREN expr TRPAREN block
			{
				$$ = new WhileStatement($3, *$5);  
			}
;

dec_case : TCASOS TLPAREN  expr TRPAREN TLBRACE block_case TRBRACE
			{
				printf(";Siii tonto\n");
			}
		;


block_case : TCASO TDOSPUNTOS expr block
			{
				printf(";Siii tonto bloque\n");
			}
			| block_case 
			;


comparison : TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE;

%%
