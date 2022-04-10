%{
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "scanner.h"
#include "ast.h"
#include "symtab.h"
#include "3ac.h"
#include "statement.h"
#include "expression.h"
#include "y.tab.h"

extern "C" int yylex();

#define TAB_SIZE 4

extern TOKEN tdata;

int col_no = 1;
int line_no = 1;
int prev_line_no = 0;
int prev_col_no = 0;
bool wspace = false;

std::stringstream text;
std::vector<std::string> code;

void commentCheck();
void updateLoc_wspace();
void updateLoc();
int main();
int yylex();
int check_type();

%}

Digit			[0-9]
Alpha			[a-zA-Z_]
Hexa			[a-fA-F0-9]
Expo			[Ee][+-]?{Digit}+
FS			    (f|F|l|L)
IS			    (u|U|l|L)*

%%
"/*"			{ wspace = true; commentCheck(); }
"//"[^\n]*\n	{ wspace = true; code.push_back(text.str()); text.clear(); text.str(std::string()); line_no++;  col_no = 1; } 

"..."			{ updateLoc(); return(ELLIPSIS); }	

"-"				{  yylval.terminal= create_terminal("-", NULL, line_no, col_no); updateLoc(); return('-'); }
"+"				{  yylval.terminal= create_terminal("+", NULL, line_no, col_no); updateLoc(); return('+'); }
"*"				{  yylval.terminal= create_terminal("*", NULL, line_no, col_no); updateLoc(); return('*'); }
"/"				{ updateLoc(); return('/'); }
"%"				{ updateLoc(); return('%'); }
"^"				{ updateLoc(); return('^'); }

"+="			{  yylval.terminal= create_terminal("+=", NULL, line_no, col_no); updateLoc(); return(ADD_ASSIGN); }
"-="			{  yylval.terminal= create_terminal("-=", NULL, line_no, col_no); updateLoc(); return(SUB_ASSIGN); }
"*="			{  yylval.terminal= create_terminal("*=", NULL, line_no, col_no); updateLoc(); return(MUL_ASSIGN); }
"/="			{  yylval.terminal= create_terminal("/=", NULL, line_no, col_no); updateLoc(); return(DIV_ASSIGN); }
"%="			{  yylval.terminal= create_terminal("%=", NULL, line_no, col_no); updateLoc(); return(MOD_ASSIGN); }
"^="			{  yylval.terminal= create_terminal("^=", NULL, line_no, col_no); updateLoc(); return(XOR_ASSIGN); }
">>="			{ yylval.terminal= create_terminal(">>=", NULL, line_no, col_no); updateLoc(); return(RIGHT_ASSIGN); }
"<<="			{ yylval.terminal= create_terminal("<<=", NULL, line_no, col_no); updateLoc(); return(LEFT_ASSIGN); }
"&="			{  yylval.terminal= create_terminal("&=", NULL, line_no, col_no); updateLoc(); return(AND_ASSIGN); }
"|="			{  yylval.terminal= create_terminal("|=", NULL, line_no, col_no); updateLoc(); return(OR_ASSIGN); }

"++"			{  yylval.terminal= create_terminal("++", NULL, line_no, col_no); updateLoc(); return(INC_OP); }
"--"			{  yylval.terminal= create_terminal("--", NULL, line_no, col_no); updateLoc(); return(DEC_OP); }

"="				{  yylval.terminal= create_terminal("=", NULL, line_no, col_no); updateLoc(); return('='); }

">>"			{  yylval.terminal= create_terminal(">>", NULL, line_no, col_no); updateLoc(); return(RIGHT_OP); }
"<<"			{  yylval.terminal= create_terminal("<<", NULL, line_no, col_no); updateLoc(); return(LEFT_OP); }
"->"			{ updateLoc(); return(PTR_OP); }
"&&"			{ updateLoc(); return(AND_OP); }
"||"			{ updateLoc(); return(OR_OP); }
"<="			{ updateLoc(); return(LE_OP); }
">="			{ updateLoc(); return(GE_OP); }
"=="			{ updateLoc(); return(EQ_OP); }
"!="			{ updateLoc(); return(NE_OP); }

"{"				{ updateLoc(); return('{'); }
"}"				{ updateLoc(); return('}'); }
"("				{ updateLoc(); return('('); }
")"				{ updateLoc(); return(')'); }
"["				{ updateLoc(); return('['); }
"]"				{ updateLoc(); return(']'); }

";"				{ updateLoc(); return(';'); }
","			    { updateLoc(); return(','); }
":"				{ updateLoc(); return(':'); }
"."				{ updateLoc(); return('.'); }
"?"				{ updateLoc(); return('?'); }


"!"				{  yylval.terminal= create_terminal("!", NULL, line_no, col_no); updateLoc(); return('!'); }
"&"				{  yylval.terminal= create_terminal("&", NULL, line_no, col_no); updateLoc(); return('&'); }
"|"				{ updateLoc(); return('|'); }
"~"				{  yylval.terminal= create_terminal("~", NULL, line_no, col_no); updateLoc(); return('~'); }
"<"				{ updateLoc(); return('<'); }
">"				{ updateLoc(); return('>'); }


"break"			{ updateLoc(); return(BREAK); }
"case"			{ updateLoc(); return(CASE); }
"char"			{ yylval.type_specifier = create_type_specifier(CHAR, line_no, col_no); updateLoc(); return(CHAR); }
"const"			{ updateLoc(); return(CONST); }
"continue"		{ updateLoc(); return(CONTINUE); }
"default"		{ updateLoc(); return(DEFAULT); }
"do"			{ updateLoc(); return(DO); }
"double"		{ yylval.type_specifier = create_type_specifier(DOUBLE, line_no, col_no); updateLoc(); return(DOUBLE); }
"else"			{ updateLoc(); return(ELSE); }
"extern"		{ updateLoc(); return(EXTERN); }
"float"			{ yylval.type_specifier = create_type_specifier(FLOAT, line_no, col_no); updateLoc(); return(FLOAT); }
"for"			{ updateLoc(); return(FOR); }
"if"			{ updateLoc(); return(IF); }
"int"			{ yylval.type_specifier = create_type_specifier(INT, line_no, col_no); updateLoc(); return(INT); }
"long"			{ yylval.type_specifier = create_type_specifier(LONG, line_no, col_no); updateLoc(); return(LONG); }
"return"		{ updateLoc(); return(RETURN); }
"short"			{ yylval.type_specifier = create_type_specifier(SHORT, line_no, col_no); updateLoc(); return(SHORT); }
"signed"		{ yylval.type_specifier = create_type_specifier(SIGNED, line_no, col_no); updateLoc(); return(SIGNED); }
"sizeof"		{ yylval.type_specifier = create_type_specifier(SIZEOF, line_no, col_no); updateLoc(); return(SIZEOF); }
"static"		{ updateLoc(); return(STATIC); }
"struct"		{ updateLoc(); return(STRUCT); }
"switch"		{ updateLoc(); return(SWITCH); }
"typedef"		{ updateLoc(); return(TYPEDEF); }
"union"			{ updateLoc(); return(UNION); }
"unsigned"		{ yylval.type_specifier = create_type_specifier(UNSIGNED, line_no, col_no); updateLoc(); return(UNSIGNED); }
"void"			{ yylval.type_specifier = create_type_specifier(VOID, line_no, col_no); updateLoc(); return(VOID); }
"while"			{ updateLoc(); return(WHILE); }

{Alpha}({Alpha}|{Digit})* { int type = check_type(); updateLoc(); return(type); }

0[xX]{Hexa}+{IS}?	    { yylval.constant = create_constant("CONSTANT HEX", yytext, line_no, col_no); updateLoc(); return(CONSTANT); }
0{Digit}+{IS}?          { yylval.constant = create_constant("CONSTANT INT", yytext, line_no, col_no); updateLoc(); return(CONSTANT); }
{Digit}+{IS}?		{ yylval.constant = create_constant("CONSTANT INT", yytext, line_no, col_no); updateLoc(); return(CONSTANT); }			
L?'(\\.|[^\\'])+'	{ updateLoc(); yylval.constant = create_constant("CONSTANT CHAR", yytext, line_no, prev_col_no); return(CONSTANT); }		

{Digit}+{Expo}{FS}? 	{ yylval.constant = create_constant("CONSTANT EXP", yytext, line_no, col_no); updateLoc(); return(CONSTANT); }		
{Digit}*"."{Digit}+({Expo})?{FS}?	{ yylval.constant = create_constant("CONSTANT FLOAT", yytext, line_no, col_no); updateLoc(); return(CONSTANT); }
{Digit}+"."{Digit}*({Expo})?{FS}?	{ yylval.constant = create_constant("CONSTANT FLOAT", yytext, line_no, col_no); updateLoc(); return(CONSTANT); }

\"(\\.|[^\\"])*\"	{ yylval.terminal = create_terminal("STRING LITERAL", yytext, line_no, col_no); updateLoc(); return(STRING_LITERAL); }     

[ \t\n\v\f ]		{ updateLoc_wspace(); }
.			{ /* ignore  characters */ }

%%

// Check multiline comments starting with /*
void commentCheck()
{
	char ch, ch1;
    while(1)
    {
        while ((ch = yyinput()) != '*' && ch != 0)
        {
            if(ch=='\n'){
                code.push_back(text.str());
                text.clear();
                text.str(std::string());
                line_no++;
            }
        }
        
        if ((ch1 = yyinput()) != '/' && ch != 0)
        {
            unput(ch1);
        }
        else break;
    }

}

void updateLoc_wspace(){
    if(!wspace){
        prev_col_no = col_no;
        prev_line_no = line_no;
    }
    wspace = true;
    int length = strlen(yytext);
    for(int i = 0; i < length; i++){
        if(yytext[i] == '\n'){
            col_no = 1;
            line_no++;
            code.push_back(text.str());
            text.clear();
            text.str(std::string());
        }
        else if(yytext[i] == '\t'){
            int chunk_filled = col_no % TAB_SIZE;
            col_no += TAB_SIZE - chunk_filled;
            text << yytext[i];
        }
        else{
            col_no++;
            text << yytext[i];
        }
    }
}

// Update the current column number and line number
void updateLoc()
{   
    if(!wspace){
        prev_col_no = col_no;
        prev_line_no = line_no;
    }
    wspace = false;
	tdata.lexeme = strdup(yytext);
    int length = strlen(yytext);
    for(int i = 0; i < length; i++){
        if(yytext[i] == '\n'){
            line_no++;
            col_no = 1;
            code.push_back(text.str());
            text.clear();
            text.str(std::string());
        }
        else if(yytext[i] == '\t'){
            int chunk_filled = col_no % TAB_SIZE;
            col_no += TAB_SIZE - chunk_filled;
            text << yytext[i];
        }
        else{
            col_no++;
            text << yytext[i];
        }

    }
}

int yywrap()
{	
	return 1;
}

int check_type(){
    if(std::string(yytext) == "NULL"){
        yylval.constant = create_constant("CONSTANT HEX", "0x0", line_no, col_no);
        yylval.constant->ConstantType = Type(VOID_T, 1, true);
        yylval.constant->ConstantType.is_pointer = true;
        return(CONSTANT);
    }
    if(std::string(yytext) == "true"){
        yylval.constant = create_constant("CONSTANT INT", "1", line_no, col_no);
        yylval.constant->ConstantType = Type(CHAR_T, 0, true);
        return(CONSTANT);
    }
    else if (std::string(yytext) == "false"){
        yylval.constant = create_constant("CONSTANT INT", "0", line_no, col_no);
        yylval.constant->ConstantType = Type(CHAR_T, 0, true);
        return(CONSTANT);
    }

    Identifier *id = new Identifier(yytext, line_no, col_no);
    yylval.identifier = id;
    return(IDENTIFIER);
}


/*
    reference: https://www.lysator.liu.se/c/ANSI-C-grammar-l.html
*/