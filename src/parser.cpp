#include <assert.h>
#include <string>
#include <sstream>
#include <iostream>

#include "ast.h"
#include "symtab.h"
#include "expression.h"
#include <scanner.h>
#include <y.tab.h>


extern FILE *yyin;
extern FILE *yyout;

Node* root = NULL;
TOKEN tdata;

// std::ofstream sym_file;
std::ofstream symbol_file;

static std::ofstream dot_file;

int main(int argc, char *argv[]) {
  FILE *fh;

  if (argc != 4){
	  std::cout << "Incorrect usage. Usage : ./bin/parser <file>.c -o <file>.dot\n";
  }
  if ((fh = fopen(argv[1], "r"))){
  	yyin = fh;
  }
  else{
	  std::cout << "Input file does not exist!";
	  exit(0);
  }

  	// std::ofstream outfile(argv[3]);
  	dot_file.open(argv[3]); // = outfile;
	// sym_file.open("symtab.csv");
	// sym_file << "Scope, Function Name, Symbol Name, Symbol, Type, Symbol Level\n";
	symbol_file.open("symtab.csv");
	symbol_file << "Scope, Function Name, Symbol Name, Symbol, Type, Symbol Level\n";
	
	std::stringstream ss;
	ss << "digraph G {\n";
	ss << "\tordering=out\n";
	dot_file << ss.str();

	root = create_non_term("translation_unit");
	
	instantiate_primitive_types();
	// setup_primitive_types();
	int abc = yyparse();
	root->dotify();
	
	assert(abc == 0);
	
	ss.str("");
	ss << "}\n";
	dot_file << ss.str();

	dot_file.close();

  return 0;
}

void file_writer(std::string s){
	dot_file << s;
}