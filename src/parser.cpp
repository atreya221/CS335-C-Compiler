#include <assert.h>
#include <string>
#include <sstream>
#include <iostream>

#include "ast.h"
#include "symtab.h"
#include "expression.h"
#include "scanner.h"
#include "statement.h"
#include "y.tab.h"

int error_flag;
std::stringstream tac_ss;
std::stringstream asm_ss;
std::stringstream dot_ss;
std::stringstream sym_ss;


extern FILE *yyin;
extern FILE *yyout;

Node * root = NULL;
TOKEN tdata;

int main(int argc, char *argv[]) {
	
	std::string input_file;
	std::string output_file = "file.s";
	if(argc == 1){
		std::cout << "./bin/codegen <file-path> [-o] <output-name>" << std::endl;
		exit(0);

	}
	else if(argc == 2){
		input_file = argv[1];
	}
	else if(argc == 4){
		if(std::string(argv[2]) == "-o"){
			input_file = argv[1];
			output_file = argv[3];
		}
		else{
			std::cout << argv[2] << std::endl;
			std::cout << "./bin/codegen <file-path> [-o] <output-name>" << std::endl;
			exit(0);

		}
	}
	else{
		std::cout << "./bin/codegen <file-path> [-o] <output-name>" << std::endl;
		exit(0);

	}

	error_flag = 0;

	FILE *fh;
	if ((fh = fopen(input_file.c_str(), "r"))){
		yyin = fh;
	}
	else{
	 	std::cout << "Input file does not exist!";
	 	exit(0);
	}

	sym_ss << "Scope, Function Name, Symbol Name, Symbol Type, Symbol Level\n";
	std::stringstream ss;
	dot_ss << "digraph G {\n";
	dot_ss << "\tordering=out\n";

	root = create_non_term("translation_unit");

	instantiate_primitive_types();

	int abc = yyparse();
	root->dotify();
	
	assert(abc == 0);

	dot_ss << "}\n";

	if(error_flag == 1){
		std::cout << "\033[1;31mCOMPILATION FAILED\033[0m\n";
		exit(0);
	}
	else{
	
		std::string file_basename = output_file.substr(0, output_file.find_last_of("."));
		
		std::ofstream dot_ofs;
		const std::string dot_fname = file_basename + ".dot";
		dot_ofs.open(dot_fname);
		dot_ofs << dot_ss.rdbuf();
		dot_ofs.close();


		std::ofstream sym_ofs;
		const std::string sym_fname = file_basename + "_symtab.csv";
		sym_ofs.open(sym_fname);
		sym_ofs << sym_ss.rdbuf();
		sym_ofs.close();

		std::ofstream tac_ofs;
		const std::string tac_fname = file_basename + ".tac";
		tac_ofs.open(tac_fname);
		tac_ofs << tac_ss.rdbuf();
		tac_ofs.close();
		
		std::ofstream asm_ofs;
		const std::string asm_fname = output_file;
		asm_ofs.open(asm_fname);
		asm_ofs << asm_ss.rdbuf();
		asm_ofs.close();

		std::stringstream ssr1;
		ssr1 << "sed -i 's/^ASM: //' " << asm_fname;
		system(ssr1.str().c_str());
		std::stringstream ssr2;
		ssr2 << "sed -i '/^xxxx/d' " << asm_fname;
		system(ssr2.str().c_str());
	}
  return 0;
}

void file_writer(std::string s){
	dot_ss << s;
}
