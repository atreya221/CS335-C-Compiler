%{
	#include <stdio.h>
	#include "ast.h"
	#include "symtab.h"
	#include "expression.h"
	void yyerror(const char *s);	
	extern "C" int yylex();
	extern Node * root;
%}

%union{
	Node * node;
	int value;

	DeclarationList * declaration_list;
		Declaration * declaration;
	
			DeclarationSpecifiers * declaration_specifiers;
				TypeSpecifier * type_specifier;
					EnumeratorList * enumerator_list;
						Enumerator * enumerator;
					StructDeclarationList * struct_declaration_list;
						StructDeclaration * struct_declaration;
						SpecifierQualifierList * specifier_qualifier_list;


			DeclaratorList * init_declarator_list;
				Declarator * declarator;
					DirectDeclarator * direct_declarator;
						Identifier * identifier;
						ParameterTypeList * parameter_type_list;
							ParameterDeclaration * parameter_declaration;
								AbstractDeclarator * abstract_declarator;
								DirectAbstractDeclarator * direct_abstract_declarator;
									Constant * constant;


					Pointer * pointer;
						TypeQualifierList * type_qualifier_list;
	
	FunctionDefinition * function_definition;

	Expression * expression;
		PrimaryExpression * primary_expression;
		AssignmentExpression * assignment_expression;
		Logical_andExpression * logical_and_expression;
		Logical_orExpression * logical_or_expression;
		UnaryExpression * unary_expression;
		CastExpression * cast_expression;
		ShiftExpression * shift_expression; 
		InclusiveorExpression * inclusive_or_expression; 
		ExclusiveorExpression * exclusive_or_expression;
		RelationalExpression * relational_expression;
		MultiplicativeExpression * multiplicative_expression;
		AdditiveExpression * additive_expression;
		ArgumentExprList * argument_expression_list;
		ConditionalExpression * conditional_expression;
		AndExpression * and_expression;
		EqualityExpression * equality_expression;  
		PostfixExpression * postfix_expression;
		StringLiteral * string_literal;

}



%token <node> '(' ')' '[' ']' '.' ',' '+' '-' '!' '&' '*' '~' '/' '%'
%token <node> '<' '>' '^' '|' ':' '?' '=' ';' '{' '}'

%token <identifier> IDENTIFIER
%token <constant> CONSTANT 
%token <string_literal> STRING_LITERAL 
%token <node> SIZEOF

/*
%type <expression> expression
%type <assignment_expression> assignment_expression
%type <logical_and_expression> logical_and_expression
%type <logical_or_expression> logical_or_expression 
%type <unary_expression> unary_expression
%type <cast_expression> cast_expression
%type <shift_expression> shift_expression 
%type <inclusive_or_expression> inclusive_or_expression 
%type <exclusive_or_expression> exclusive_or_expression
%type <primary_expression> primary_expression
%type <relational_expression> relational_expression
%type <multiplicative_expression> multiplicative_expression
%type <additive_expression> additive_expression
%type <argument_expression_list> argument_expression_list
%type <conditional_expression> conditional_expression
%type <and_expression> and_expression
%type <equality_expression> equality_expression  
%type <postfix_expression> postfix_expression
*/

%type <expression> expression
%type <expression> assignment_expression
%type <expression> logical_and_expression
%type <expression> logical_or_expression 
%type <expression> unary_expression
%type <expression> cast_expression
%type <expression> relational_expression
%type <expression> shift_expression 
%type <expression> inclusive_or_expression 
%type <expression> exclusive_or_expression
%type <expression> additive_expression
%type <expression> argument_expression_list
%type <expression> multiplicative_expression
%type <expression> and_expression
%type <expression> equality_expression  
%type <expression> conditional_expression
%type <expression> postfix_expression
%type <expression> primary_expression


%token <node> PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token <node> AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token <node> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token <node> XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token <value> TYPEDEF EXTERN STATIC AUTO REGISTER
%token <value> SIGNED UNSIGNED CHAR SHORT INT LONG FLOAT DOUBLE VOID
%token <value> CONST VOLATILE
%token <value> STRUCT UNION ENUM ELLIPSIS

%token <node> CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN


%type <node> constant_expression
%type <node> unary_operator assignment_operator

%type <declaration_list> declaration_list
%type <declaration> declaration
%type <declaration_specifiers> declaration_specifiers 
%type <type_specifier> type_specifier 

%type <enumerator_list> enumerator_list
%type <enumerator> enumerator
%type<type_specifier> enum_specifier

%type <struct_declaration_list> struct_declaration_list
%type <type_specifier> struct_or_union_specifier
%type <value> struct_or_union  
%type <struct_declaration> struct_declaration
%type<specifier_qualifier_list> specifier_qualifier_list 

%type <init_declarator_list> struct_declarator_list
%type <declarator> struct_declarator

%type <init_declarator_list> init_declarator_list  
%type <declarator> init_declarator declarator
%type <pointer> pointer
%type <direct_declarator> direct_declarator

%type <type_qualifier_list> type_qualifier_list
%type <value> type_qualifier
%type <value> storage_class_specifier 

%type <function_definition> function_definition function_declaration

%type <node> type_name initializer initializer_list
%type <parameter_type_list> parameter_type_list parameter_list
%type <parameter_declaration> parameter_declaration 
%type <abstract_declarator> abstract_declarator
%type <direct_abstract_declarator>  direct_abstract_declarator 


/*%type <node> identifier_list */

%type <node> statement labeled_statement compound_statement statement_list expression_statement selection_statement iteration_statement jump_statement

%type <node> translation_unit external_declaration 

%start translation_unit
%%



/* 


	REVISIT THIS WHEN ADDING EXPRESSION.CPP to change the expression add_to_declaration_list


*/


primary_expression
	: IDENTIFIER		{ $$ = create_primary_identifier($1); } 
	| CONSTANT		{ 
						PrimaryExpression *P = new PrimaryExpression();
    					P->isTerminal = 2;
    					P->Cval = $1;
						P->type = $1->getConstantType();
						P->name = "primary_expression";
						P->add_child( $1 );
						$$ = P;
	 		}
	| STRING_LITERAL	{ 
							PrimaryExpression *P = new PrimaryExpression();
							P->isTerminal = 3;
							P->Sval = $1;
							P->type.typeIndex = PrimitiveTypes::U_CHAR_T;
							P->type.ptr_level = 1;

							P->name = "primary_expression";
							P->add_child( $1 );
							$$ = P;
						} 
	| '(' expression ')'	{ $$ = create_primary_expression($2); } 
	;

postfix_expression
	: primary_expression	{ $$ = $1; } 
	| postfix_expression '[' expression ']'	{ $$ = create_postfix_expr_arr( $1, $3 ); } 
/*	| postfix_expression '(' ')'	{ $$ = create_non_term("FUNCTION CALL", $1 ); } */
	| IDENTIFIER '(' ')'	{ 
								PostfixExpression *P = new PostfixExpression();
								P->name = "FUNCTION CALL";
								P->add_child( $1 );
								$$ = P;
							} 
	| IDENTIFIER '(' argument_expression_list ')'	{ 
		PostfixExpression *P = new PostfixExpression(); 
		P->name = "FUNCTION CALL";
		P->add_children( $1, $3 );
		$$ = P;
	} 
/*	| postfix_expression '(' argument_expression_list ')'	{ create_non_term("FUNCTION CALL ARGS", $1, $3 ); } */
	| postfix_expression '.' IDENTIFIER	{ $$ = create_postfix_expr_struct(".", $1, $3); } 
	| postfix_expression PTR_OP IDENTIFIER	{ $$ = create_postfix_expr_struct("->", $1, $3); } 
	| postfix_expression INC_OP	{ $$ = create_postfix_expr_ido("++", $1); } 
	| postfix_expression DEC_OP	{ $$ = create_postfix_expr_ido("--", $1); } 
	;

argument_expression_list
	: assignment_expression		{ 
		ArgumentExprList *P = new ArgumentExprList();
		P->op2 = $1;
		P->name = "assignment_expression";
		P->add_child( $1 );
		$$ = P;
	} 
	| argument_expression_list ',' assignment_expression	{ 
		ArgumentExprList *P = new ArgumentExprList();
		P->op1 = $1;
		P->op2 = $3;

		P->name = "assignment_expression_list";
		P->add_children($1, $3);
		$$ = P;
	} 
	;

unary_expression
	: postfix_expression			    { $$ = $1; } 
	| INC_OP unary_expression		    { $$ = create_unary_expression_ue("++", $2); } 
	| DEC_OP unary_expression		    { $$ = create_unary_expression_ue("--", $2); } 
	| unary_operator cast_expression	{ $$=create_unary_expression_cast($1,$2); } 
	| SIZEOF unary_expression		    { $$ = create_unary_expression_ue("sizeof", $2); } 
	| SIZEOF '(' type_name ')'		    {
											UnaryExpression *U = new UnaryExpression();
											string u_op = "SIZEOF type_name";
											Node *n_op = create_non_term( ( u_op ).c_str() );
											U->name = u_op;
											U->add_children( n_op, $3 );
											$$ = U;
										} 
	;

unary_operator
	: '&'	{ $$ = create_non_term("&"); }
	| '*'	{ $$ = create_non_term("*"); }
	| '+'	{ $$ = create_non_term("+"); }
	| '-'	{ $$ = create_non_term("-"); }
	| '~'	{ $$ = create_non_term("~"); }
	| '!'	{ $$ = create_non_term("!"); }
	;

cast_expression
	: unary_expression			{ $$ = $1; }
	| '(' type_name ')' cast_expression	{
											CastExpression *P = new CastExpression();
											P->op1 = $4;
											P->type = $4->type;
											P->name = "cast_expression";	P->add_children( $2, $4 );
											$$ = P; 
										}
	;

multiplicative_expression
	: cast_expression				{ $$ = $1; }
	| multiplicative_expression '*' cast_expression	{ $$ = create_multiplicative_expression("*", $1, $3); }
	| multiplicative_expression '/' cast_expression	{ $$ = create_multiplicative_expression("/", $1, $3); }
	| multiplicative_expression '%' cast_expression	{ $$ = create_multiplicative_expression("%", $1, $3); }
	;

additive_expression
	: multiplicative_expression				{ $$ = $1; }
	| additive_expression '+' multiplicative_expression	{ $$ = create_additive_expression("+", $1, $3); }
	| additive_expression '-' multiplicative_expression	{ $$ = create_additive_expression("-", $1, $3); }
	;

shift_expression
	: additive_expression				{ $$ = $1; }
	| shift_expression LEFT_OP additive_expression	{ $$ = create_shift_expression(">>", $1, $3); }
	| shift_expression RIGHT_OP additive_expression	{ $$ = create_shift_expression("<<", $1, $3); }
	;

relational_expression
	: shift_expression				{ $$ = $1; }
	| relational_expression '<' shift_expression	{ $$ = create_relational_expression("<", $1, $3); }
	| relational_expression '>' shift_expression	{ $$ = create_relational_expression(">", $1, $3); }
	| relational_expression LE_OP shift_expression	{ $$ = create_relational_expression("<=", $1, $3); }
	| relational_expression GE_OP shift_expression	{ $$ = create_relational_expression(">=", $1, $3); }
	;

equality_expression
	: relational_expression					{ $$ = $1; }
	| equality_expression EQ_OP relational_expression	{ $$ = create_equality_expression("==", $1, $3); }
	| equality_expression NE_OP relational_expression	{ $$ = create_equality_expression("!=", $1, $3); }
	;

and_expression
	: equality_expression				{ $$ = $1; }
	| and_expression '&' equality_expression	{ $$ = create_and_expression("&", $1, $3); }
	;

exclusive_or_expression
	: and_expression				{ $$ = $1; }
	| exclusive_or_expression '^' and_expression	{ $$ = create_exclusive_or_expression("^", $1, $3); }
	;

inclusive_or_expression
	: exclusive_or_expression				{ $$ = $1; }
	| inclusive_or_expression '|' exclusive_or_expression	{ $$ = create_inclusive_or_expression("|", $1, $3); }
    ;

logical_and_expression
	: inclusive_or_expression				{ $$ = $1; }
	| logical_and_expression AND_OP inclusive_or_expression	{ $$ = create_logical_and_expression("&&", $1, $3); }
	;

logical_or_expression
	: logical_and_expression				{ $$ = $1; }
	| logical_or_expression OR_OP logical_and_expression	{ $$ = create_logical_or_expression("||", $1, $3); }
	;

conditional_expression
	: logical_or_expression							{ $$ = $1; }
	| logical_or_expression '?' expression ':' conditional_expression	{ $$ = create_conditional_expression("?:", $1, $3, $5);	}
	;

assignment_expression
	: conditional_expression					 { $$ = $1; }
	| unary_expression assignment_operator assignment_expression	 { $$=create_assignment_expression($1,$2,$3); }
	;

assignment_operator
	: '='		 { $$ = create_non_term("="); }
	| MUL_ASSIGN	 { $$ = create_non_term("*="); }
	| DIV_ASSIGN	 { $$ = create_non_term("/="); }
	| MOD_ASSIGN	 { $$ = create_non_term("%="); }
	| ADD_ASSIGN	 { $$ = create_non_term("+="); }
	| SUB_ASSIGN	 { $$ = create_non_term("-="); }
	| LEFT_ASSIGN	 { $$ = create_non_term("<<="); }
	| RIGHT_ASSIGN	 { $$ = create_non_term(">>="); }
	| AND_ASSIGN	 { $$ = create_non_term("&="); }
	| XOR_ASSIGN	 { $$ = create_non_term("^="); }
	| OR_ASSIGN	 { $$ = create_non_term("|="); }
	;

expression
	: assignment_expression			 { $$ = $1; }
	| expression ',' assignment_expression	{ $$ = create_toplevel_expression($1, $3); }
	;

/* Revisit this */ 
constant_expression
	: conditional_expression	 { $$ = $1; }
	;


declaration
	: declaration_specifiers ';'	 { $$ = new_declaration($1, NULL); }
	| declaration_specifiers init_declarator_list ';' { $$ = new_declaration($1, $2); }
	;

/* These are types */
declaration_specifiers
	: storage_class_specifier	{ 	
									$$ = new DeclarationSpecifiers();
									$$->storage_class.push_back($1);
								}
	| storage_class_specifier declaration_specifiers	{ $2->storage_class.push_back($1);
		$$ = $2; }
	| type_specifier	{
							$$ = new DeclarationSpecifiers();
							$$->type_specifier.push_back($1);
							$$->add_child($1);
						}
	| type_specifier declaration_specifiers	{ 
												$2->type_specifier.push_back($1);
												$2->add_child($1);
												$$ = $2;	
											}
	| type_qualifier	{
							$$ = new DeclarationSpecifiers();
							$$->type_qualifier.push_back($1);
						}
	| type_qualifier declaration_specifiers	 { $2->type_qualifier.push_back($1); $$ = $2; }
	;

init_declarator_list
	: init_declarator	 { $$ = create_init_declarator_list($1); }
	| init_declarator_list ',' init_declarator	 { $$ = add_to_init_declarator_list($1, $3); }
	;

init_declarator
	: declarator	 { $$ = $1; }
	| declarator '=' initializer	{
										$1->init_expr = $3;
										Identifier *id = new Identifier($1->id->value.c_str());
										$1->add_child(create_non_term("=", id, $3));
										$$ = $1;
									}
	;

storage_class_specifier
	:  TYPEDEF	{ $$ = TYPEDEF; }
/*	|  EXTERN	{ $$ = EXTERN; } */
/*	|  STATIC	{ $$ = STATIC; } */
/*	|  AUTO		{ $$ = AUTO; } */
/*	|  REGISTER	{ $$ = REGISTER; } */
	;

type_specifier
	:  VOID		{ $$ = create_type_specifier(VOID); }
	|  CHAR		{ $$ = create_type_specifier(CHAR); }
	|  SHORT	{ $$ = create_type_specifier(SHORT); }
	|  INT		{ $$ = create_type_specifier(INT); }
	|  LONG		{ $$ = create_type_specifier(LONG); }
	|  FLOAT	{ $$ = create_type_specifier(FLOAT); }
	|  DOUBLE	{ $$ = create_type_specifier(DOUBLE); }
	|  SIGNED	{ $$ = create_type_specifier(SIGNED); }
	|  UNSIGNED	{ $$ = create_type_specifier(UNSIGNED); }
	|  struct_or_union_specifier	{ $$ = $1; }
	|  enum_specifier		{ $$ = $1; }
	|  TYPE_NAME	{ $$ = create_type_specifier(TYPE_NAME); }
	;

struct_or_union_specifier
	:  struct_or_union IDENTIFIER '{' struct_declaration_list '}'	 { $$ =  create_type_specifier($1, $2, $4); }
	|  struct_or_union '{' struct_declaration_list '}'	 { $$ = create_type_specifier($1, NULL, $3); }
	|  struct_or_union IDENTIFIER	 { $$ = create_type_specifier($1, $2, (StructDeclarationList *) NULL); }
	;

struct_or_union
	:  STRUCT	 { $$ = STRUCT; }
	|  UNION	 { $$ = UNION; }
	;

struct_declaration_list
	:  struct_declaration	{
								$$ = new StructDeclarationList();
								$$->struct_declaration_list.push_back($1);
								$$->add_child($1);
							}
	|  struct_declaration_list struct_declaration	{
														$1->struct_declaration_list.push_back($2);
														$1->add_child($2);
														$$ = $1;
													}
	;

struct_declaration
	:  specifier_qualifier_list struct_declarator_list ';'	{
																$$ = new StructDeclaration($1, $2);
																$$->add_children($1, $2);
																$1->create_type();
															}
	;

specifier_qualifier_list
	:  type_specifier specifier_qualifier_list	{
													$2->type_specifiers.push_back($1);
													$2->add_child($1);
													$$ = $2;
												}
	|  type_specifier	{ 
							$$ = new SpecifierQualifierList();
							$$->type_specifiers.push_back($1);
							$$->add_child($1);
						}
	|  type_qualifier specifier_qualifier_list	{
													$2->type_qualifiers.push_back($1);
													$$ = $2;
												}
	|  type_qualifier	{ 
							$$ = new SpecifierQualifierList();
							$$->type_qualifiers.push_back($1);
						}
	;

struct_declarator_list
	:  struct_declarator	 { $$ = create_init_declarator_list($1); }
	|  struct_declarator_list ',' struct_declarator	 { $$ = add_to_init_declarator_list($1, $3); }
	;

struct_declarator
	:  declarator	 { $$ = $1; }
/*	
	To visit later
	|  ':' constant_expression	 { $$ = create_non_term(":", $2); }
	|  declarator ':' constant_expression	 { $$ = create_non_term(":", $1, $3); }
*/
	;

enum_specifier
	:  ENUM '{' enumerator_list '}'	 { $$ = create_type_specifier(ENUM, NULL, $3); }
	|  ENUM IDENTIFIER '{' enumerator_list '}'	 { $$ = create_type_specifier(ENUM, $2,  $4); }
	|  ENUM IDENTIFIER	 { $$ = create_type_specifier( ENUM, $2, (EnumeratorList *) NULL ); }
	;

enumerator_list
	:  enumerator	{ 
						$$ = new EnumeratorList();
						$$->enumerator_list.push_back($1);
						$$->add_child($1);
					}
	|  enumerator_list ',' enumerator	{ 
											$1->enumerator_list.push_back($3);
											$1->add_child($3);
											$$ = $1;
										}
	;

enumerator
	:  IDENTIFIER	 { $$ = create_enumerator($1, NULL); }
	|  IDENTIFIER '=' constant_expression	 { $$ = create_enumerator($1, $3); }
	;

type_qualifier
	:  CONST	 { $$ = CONST; }
	|  VOLATILE	 { $$ = VOLATILE; }
	;

declarator
	:  pointer direct_declarator	 { $$ = create_declarator($1, $2); }
	|  direct_declarator	 { $$ = create_declarator(NULL, $1); }
	;

direct_declarator
	:  IDENTIFIER	 					 { $$ = create_dir_declarator_id( ID , $1 ); }
	|  '(' declarator ')'	 				 { $$ = create_dir_declarator_dec( DECLARATOR, $2 ); }
	/*|  direct_declarator '[' constant_expression ']'	 { $$ = create_dir_declarator_arr( ARRAY, $1, $3 ); }*/
	|  direct_declarator '[' CONSTANT ']'	 { $$ = create_dir_declarator_arr( ARRAY, $1, $3 ); }
	|  direct_declarator '[' ']'	 			 { $$ = create_dir_declarator_arr( ARRAY, $1, NULL ); }
	|  direct_declarator '(' parameter_type_list ')'	 { $$ = create_dir_declarator_fun( FUNCTION, $1, $3 ); }
	|  direct_declarator '(' ')'				 { $$ = create_dir_declarator_fun( FUNCTION, $1, NULL ); }
	;

pointer
	:  '*'	 				{ $$ = create_pointer(); }
	|  '*' type_qualifier_list	 	{ $$ = create_pointer( $2, NULL ); }
	|  '*' pointer	 			{ $$ = create_pointer( NULL, $2 ); }
	|  '*' type_qualifier_list pointer	{ $$ = create_pointer( $2, $3 ); }
	;

type_qualifier_list
	:  type_qualifier	 { $$ = new TypeQualifierList(); $$->append_to_list($1); }
	|  type_qualifier_list type_qualifier	 { $1->append_to_list($2); $$ = $1; }
	;

parameter_type_list
	:  parameter_list	 { $$ = $1; }
	|  parameter_list ',' ELLIPSIS	{ 
										$1->has_ellipsis = true;
										$1->add_child(create_terminal("...", NULL));
										$$ = $1; 
									}
	;

parameter_list
	:  parameter_declaration	{ 
									$$ = new ParameterTypeList();
									$$->param_list.push_back($1);
									$$->add_child($1);
								}
	|  parameter_list ',' parameter_declaration	{ 
													$1->param_list.push_back($3);
													$1->add_child($3);
													$$ = $1;
												}
	;

parameter_declaration
	:  declaration_specifiers declarator	 	{ $$ = create_parameter_declaration( $1, $2, NULL ); } 
	|  declaration_specifiers abstract_declarator	{ $$ = create_parameter_declaration( $1, NULL, $2 ); } 
	|  declaration_specifiers	 		{ $$ = create_parameter_declaration( $1, NULL, NULL ); } 
	;

/*
identifier_list
	:  IDENTIFIER	 { $$ = create_terminal("IDENTIFIER",NULL); }
	|  identifier_list ',' IDENTIFIER	 { $$ = create_non_term("identifier_list", $1, create_terminal("IDENTIFIER",NULL)); }
	;
*/

type_name
	:  specifier_qualifier_list	 { $$ = create_non_term("type_name", $1); }
	|  specifier_qualifier_list abstract_declarator	 { $$ = create_non_term("type_name", $1, $2); }
	;

abstract_declarator
	:  pointer	 			{ $$ = create_abstract_declarator( $1, NULL ); } 
	|  direct_abstract_declarator	 	{ $$ = create_abstract_declarator( NULL, $1 ); } 
	|  pointer direct_abstract_declarator	{ $$ = create_abstract_declarator( $1, $2 ); } 
	;

direct_abstract_declarator
	:  '(' abstract_declarator ')'					{ $$ = create_direct_abstract_declarator( ABSTRACT, $2 ); }
	|  '[' ']'							{ $$ = create_direct_abstract_declarator( SQUARE ); } 
	/*|  '[' constant_expression ']'	 				{ $$ = create_direct_abstract_declarator( SQUARE, NULL, $2 ); } */
	|  '[' CONSTANT ']'	 				{ $$ = create_direct_abstract_declarator( SQUARE, NULL, $2 ); } 
	|  direct_abstract_declarator '[' ']'				{ $$ = create_direct_abstract_declarator( SQUARE, $1 ); } 
	|  direct_abstract_declarator '[' constant_expression ']'	{ $$ = create_direct_abstract_declarator( SQUARE, $1, $3 ); } 
	|  '(' ')'	 						{ $$ = create_direct_abstract_declarator( ROUND ); } 
	|  '(' parameter_type_list ')'	 				{ $$ = create_direct_abstract_declarator( ROUND, NULL, $2 ); } 
	|  direct_abstract_declarator '(' ')'	 			{ $$ = create_direct_abstract_declarator( ROUND, $1 ); } 
	|  direct_abstract_declarator '(' parameter_type_list ')'	{ $$ = create_direct_abstract_declarator( ROUND, $1, $3 ); } 
	;

initializer
	:  assignment_expression	 { $$ = $1; }
	|  '{' initializer_list '}'	 { $$ = create_non_term("initializer_list", $2); }
	|  '{' initializer_list ',' '}'	 { $$ = create_non_term("initializer_list", $2); }
	;

initializer_list
	:  initializer	 { $$ = create_non_term("initializer", $1); }
	|  initializer_list ',' initializer	 { $$ = create_non_term("initializer_list", $1, $3); }
	;

statement
	:  labeled_statement	 { $$ = $1; }
	|  compound_statement	 { $$ = $1; }
	|  expression_statement	 { $$ = create_non_term("expression_statement", $1); }
	|  selection_statement	 { $$ = create_non_term("selection_statement", $1); }
	|  iteration_statement	 { $$ = create_non_term("iteration_statement", $1); }
	|  jump_statement	 { $$ = create_non_term("jump_statement", $1); }
	;

labeled_statement
	:  IDENTIFIER ':' statement	 { $$ = create_non_term("labeled_statement", $1, $3); }
	|  CASE constant_expression ':' statement	 { $$ = create_non_term("CASE", $2, $4); }
	|  DEFAULT ':' statement	 { $$ = create_non_term("DEFAULT", $2); }
	;

compound_statement
	:  '{' '}'	 { $$ = NULL; }
	|  '{' statement_list '}'	 { $$ = create_non_term("compound_statement", $2); }
	|  '{' declaration_list '}'	 { local_symbol_table.clear_current_level(); $$ = create_non_term("compound_statement", $2); }
	|  '{' declaration_list statement_list '}'	{ local_symbol_table.clear_current_level(); $$ = create_non_term("compound_statement", $2, $3); }
	;


declaration_list
	:  declaration	{
						$$ = new DeclarationList();
						$$->declarations.push_back($1);
						$$->add_child($1);
						local_symbol_table.increase_level();
						$1->add_to_symbol_table(local_symbol_table);
					}
	|  declaration_list declaration	{
		$1->declarations.push_back($2);
		$1->add_child($2);
		$2->add_to_symbol_table(local_symbol_table);
		$$ = $1;
	}
	;


statement_list
	:  statement	 { $$ = $1; }
	|  statement_list statement	 { $$ = create_non_term("statement_list", $1, $2); }
	;

expression_statement
	:  ';'	 { $$ = create_terminal("EMPTY_STMT",NULL); }
	|  expression ';'	 { $$ = $1; }
	;

selection_statement
	:  IF '(' expression ')' statement	 { $$ = create_non_term("IF", $3, $5); }
	|  IF '(' expression ')' statement ELSE statement	 { $$ = create_non_term("IF ELSE", $3, $5, $7); }
	|  SWITCH '(' expression ')' statement	 { $$ = create_non_term("SWITCH", $3, $5); }
	;

iteration_statement
	:  WHILE '(' expression ')' statement	 { $$ = create_non_term("WHILE", $3, $5); }
	|  DO statement WHILE '(' expression ')' ';'	 { $$ = create_non_term("DO WHILE", $5, $2); }
	|  FOR '(' expression_statement expression_statement ')' statement	 { $$ = create_non_term("FOR", $3, $4, NULL, $6); }
	|  FOR '(' expression_statement expression_statement expression ')' statement	 { $$ = create_non_term("FOR", $3, $4, $5, $7); }
	;

jump_statement
	:  GOTO IDENTIFIER ';'	 { $$ = create_non_term("GOTO", $2); }
	|  CONTINUE ';'	 { $$ = create_non_term("CONTINUE"); }
	|  BREAK ';'	 { $$ = create_non_term("BREAK"); }
	|  RETURN ';'	 { $$ = create_non_term("RETURN"); }
	|  RETURN expression ';'	 { $$ = create_non_term("RETURN", $2); }
	;

translation_unit
	:  external_declaration	 { root->add_child(create_non_term("external_declaration", $1)); }
	|  translation_unit external_declaration	 { root->add_child(create_non_term("external_declaration", $2)); }
	;

external_declaration
	:  function_definition	 { $$ = $1; }
	|  declaration	 { $1->add_to_symbol_table(global_symbol_table); $$ = $1; }
	;

function_declaration
	: declaration_specifiers declarator	{ 
											$$ = new FunctionDefinition($1, $2, NULL);
											$$->add_children($1, $2, NULL);
											$1->create_type();
											global_symbol_table.add_symbol($1, $2);
											local_symbol_table.add_function($1, $2);
										}
	;

function_definition
	:  function_declaration compound_statement	{
													$1->compound_statement = $2;
													$1->add_child($2);
													$$ = $1;
												}
	;
/*	:  declaration_specifiers declarator compound_statement	 { $$ = create_function_defintion($1, $2, $3); }*/
/*	|  declarator compound_statement	 { $$ = create_non_term("function_definition", $2); }*/

%%
#include <stdio.h>

extern char yytext[];
extern int col_no;

void yyerror(const char *s)
{
	fflush(stdout);
	printf("\n%*s\n%*s\n", col_no, "^", col_no, s);
}
