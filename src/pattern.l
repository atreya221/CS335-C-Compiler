%{
#include <stdio.h>

#include "scanner.h"
#include "ast.h"
#include "symtab.h"
#include "expression.h"
#include "y.tab.h"

extern "C" int yylex();

#define TAB_SIZE 4

extern TOKEN tdata;

int col_no = 1;
int line_no = 1;

void commentCheck();
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
"/*"			{ commentCheck(); }
"//"[^\n]*\n	{ line_no++;  col_no = 1; } 
"#"[^\n]*\n     { line_no++; col_no = 1;}

"..."			{ updateLoc(); return(ELLIPSIS); }	

"-"				{ updateLoc(); return('-'); }
"+"				{ updateLoc(); return('+'); }
"*"				{ updateLoc(); return('*'); }
"/"				{ updateLoc(); return('/'); }
"%"				{ updateLoc(); return('%'); }
"^"				{ updateLoc(); return('^'); }

"+="			{ updateLoc(); return(ADD_ASSIGN); }
"-="			{ updateLoc(); return(SUB_ASSIGN); }
"*="			{ updateLoc(); return(MUL_ASSIGN); }
"/="			{ updateLoc(); return(DIV_ASSIGN); }
"%="			{ updateLoc(); return(MOD_ASSIGN); }
"^="			{ updateLoc(); return(XOR_ASSIGN); }
">>="			{ updateLoc(); return(RIGHT_ASSIGN); }
"<<="			{ updateLoc(); return(LEFT_ASSIGN); }
"&="			{ updateLoc(); return(AND_ASSIGN); }
"|="			{ updateLoc(); return(OR_ASSIGN); }

"++"			{ updateLoc(); return(INC_OP); }
"--"			{ updateLoc(); return(DEC_OP); }

"="				{ updateLoc(); return('='); }

">>"			{ updateLoc(); return(RIGHT_OP); }
"<<"			{ updateLoc(); return(LEFT_OP); }
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


"!"				{ updateLoc(); return('!'); }
"&"				{ updateLoc(); return('&'); }
"|"				{ updateLoc(); return('|'); }
"~"				{ updateLoc(); return('~'); }
"<"				{ updateLoc(); return('<'); }
">"				{ updateLoc(); return('>'); }


"break"			{ updateLoc(); return(BREAK); }
"case"			{ updateLoc(); return(CASE); }
"char"			{ updateLoc(); return(CHAR); }
"const"			{ updateLoc(); return(CONST); }
"continue"		{ updateLoc(); return(CONTINUE); }
"default"		{ updateLoc(); return(DEFAULT); }
"do"			{ updateLoc(); return(DO); }
"double"		{ updateLoc(); return(DOUBLE); }
"else"			{ updateLoc(); return(ELSE); }
"extern"		{ updateLoc(); return(EXTERN); }
"float"			{ updateLoc(); return(FLOAT); }
"for"			{ updateLoc(); return(FOR); }
"if"			{ updateLoc(); return(IF); }
"int"			{ updateLoc(); return(INT); }
"long"			{ updateLoc(); return(LONG); }
"return"		{ updateLoc(); return(RETURN); }
"short"			{ updateLoc(); return(SHORT); }
"signed"		{ updateLoc(); return(SIGNED); }
"sizeof"		{ updateLoc(); return(SIZEOF); }
"static"		{ updateLoc(); return(STATIC); }
"struct"		{ updateLoc(); return(STRUCT); }
"switch"		{ updateLoc(); return(SWITCH); }
"typedef"		{ updateLoc(); return(TYPEDEF); }
"union"			{ updateLoc(); return(UNION); }
"unsigned"		{ updateLoc(); return(UNSIGNED); }
"void"			{ updateLoc(); return(VOID); }
"while"			{ updateLoc(); return(WHILE); }

{Alpha}({Alpha}|{Digit})* {updateLoc(); return(check_type()); }

0[xX]{Hexa}+{IS}?	    { updateLoc(); yylval.node = create_terminal("CONSTANT HEX", yytext); return(CONSTANT); }
0{Digit}+{IS}?          { updateLoc(); yylval.node = create_terminal("CONSTANT INT", yytext); return(CONSTANT); }
{Digit}+{IS}?		{ updateLoc(); yylval.node = create_terminal("CONSTANT INT", yytext); return(CONSTANT); }			
L?'(\\.|[^\\'])+'	{ updateLoc(); yylval.node = create_terminal("CONSTANT", yytext); return(CONSTANT); }		

{Digit}+{Expo}{FS}? 	{ updateLoc(); yylval.node = create_terminal("CONSTANT EXP", yytext); return(CONSTANT); }		
{Digit}*"."{Digit}+({Expo})?{FS}?	{ updateLoc(); yylval.node = create_terminal("CONSTANT FLOAT", yytext); return(CONSTANT); }
{Digit}+"."{Digit}*({Expo})?{FS}?	{ updateLoc(); yylval.node = create_terminal("CONSTANT FLOAT", yytext); return(CONSTANT); }

\"(\\.|[^\\"])*\"	{ updateLoc(); return(STRING_LITERAL); }     

[ \t\n]		{ updateLoc(); }
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
            if(ch=='\n')
                line_no++; 
        }
        
        if ((ch1 = yyinput()) != '/' && ch != 0)
        {
            unput(ch1);
        }
        else break;
    }

}

// Update the current column number and line number
void updateLoc()
{
	tdata.col_no = col_no;
	tdata.line_no = line_no;
	tdata.lexeme = strdup(yytext);
    int length = strlen(yytext);
    for(int i = 0; i < length; i++){
        if(yytext[i] == '\n'){
            line_no++;
            col_no = 1;
        }
        else if(yytext[i] == '\t'){
            int chunk_filled = col_no % TAB_SIZE;
            col_no += TAB_SIZE - chunk_filled;
        }
        else{
            col_no++;
        }

    }
}

int yywrap()
{	
	return 1;
}

int check_type(){
    // if (yytext == type_name) return(TYPE_NAME);
    //else return IDENTIFIER

    Identifier * identifier = new Identifier(yytext);
    yylval.identifier = identifier;
    return(IDENTIFIER);
}


/*
    reference: https://www.lysator.liu.se/c/ANSI-C-grammar-l.html
*/