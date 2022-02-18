#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "scanner.h"
#include "y.tab.h"


extern FILE *yyin;
extern FILE *yyout;

TOKEN tdata;

int main(int argc, char *argv[]) {
    FILE *fh;

    if (argc != 2){
        printf("Incorrect usage. Usage : ./bin/parser <file>.c\n");
        exit(0);
    }
    if ((fh = fopen(argv[1], "r"))){
        yyin = fh;
    }
    else{
        printf("Input file does not exist!");
        exit(0);
    }

	int abc = yyparse();
	assert(abc == 0);

    return 0;
}