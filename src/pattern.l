%{
#include <stdio.h>
#include "pattern.h"
#include "scanner.h"

#define TAB_SIZE 4

extern TOKEN tdata;

int col_no = 1;
int line_no = 1;

void commentCheck();
void updateLoc();
int main();
int yylex();
int yywrap();

%}

Digit			[0-9]
Alpha			[a-zA-Z_]
Hexa			[a-fA-F0-9]
Expo			[Ee][+-]?{Digit}+
FS		    	(f|F|l|Alpha)	            
IS		    	(u|U|l|Alpha)*              

%%
"/*"			{ commentCheck(); }
"//"[^\n]*\n	{ line_no++;  }     

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
"FILE"          { updateLoc(); return(FILE_TYPE);}
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

{Alpha}({Alpha}|{Digit})*		{ updateLoc(); return(IDENTIFIER); }

0[xX]{Hexa}+{IS}?	    { updateLoc(); return(CONSTANT); }
0{Digit}+{IS}?		{ updateLoc(); return(CONSTANT); }			
{Digit}+{IS}?		{ updateLoc(); return(CONSTANT); }			
Alpha?'(\\.|[^\\'])+'	{ updateLoc(); return(CONSTANT); }		

{Digit}+{Expo}{FS}?		{ updateLoc(); return(CONSTANT); }		
{Digit}*"."{Digit}+({Expo})?{FS}?	{ updateLoc(); return(CONSTANT); }
{Digit}+"."{Digit}*({Expo})?{FS}?	{ updateLoc(); return(CONSTANT); }

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
        while ((ch = input()) != '*' && ch != 0)
        {
            if(ch=='\n')
                line_no++; 
        }
        
        if ((ch1 = input()) != '/' && ch != 0)
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

    for (int i = 0; yytext[i] != '\0'; i++) {
		if (yytext[i] == '\n'){
            col_no = 1;
			line_no++;
        }
		else if (yytext[i] == '\t'){
            int chunk_filled = col_no % TAB_SIZE;
            col_no += TAB_SIZE - chunk_filled;
        }
		else
			col_no++;	
	}
}

int yywrap()
{	
	return 1;
}