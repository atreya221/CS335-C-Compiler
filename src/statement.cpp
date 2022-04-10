#include <algorithm>
#include <assert.h>
#include <iostream>
#include <list>
#include <sstream>
#include <vector>
#include <utility>
#include <string>
#include <iterator>

#include "3ac.h"
#include "ast.h"
#include "symtab.h"
#include "expression.h"
#include "statement.h"
#include "y.tab.h"

using namespace std;

map<string,vector<GoTo*>&> goto_identifier;
map<string,Label*> label_identifier;
map<string, Label*> switch_label;


Label* switch_temp = nullptr;
Type* switch_type = nullptr;
//####################
//STATEMENT 
//####################

Statement *create_selection_statement_if( Expression *expression1, GoTo * _false, Label * label1, Statement *statement1, GoTo * _goto, Label * label2, Statement *statement2 ){
    SelectionStatement* Statement = new SelectionStatement();
	if( statement2 != nullptr ) {
		assert( _goto != nullptr );
		assert( label2 != nullptr );
		
		Statement->name = "IF ELSE";
    	Statement->add_children( expression1, statement1, statement2 );
		backpatch( expression1->truelist, label1 );
		_false->set_res( expression1->res ); 
		expression1->falselist.push_back( _false );
		backpatch( expression1->falselist, label2 );

		if ( statement1 != nullptr ) {
			append(Statement->break_vector,statement1->break_vector);
			append(Statement->continue_vector,statement1->continue_vector);
			append(Statement->case_vector,statement1->case_vector);
			append(Statement->next_vector,statement1->next_vector);
			append(Statement->return_vector,statement1->return_vector);
		}
		Statement->next_vector.push_back( _goto );
		append(Statement->break_vector,statement2->break_vector);
		append(Statement->continue_vector,statement2->continue_vector);
		append(Statement->case_vector,statement2->case_vector);
		append(Statement->next_vector,statement2->next_vector);
		append(Statement->return_vector,statement2->return_vector);
    }
	else{
		Statement->name = "IF";
        Statement->add_children( expression1, statement1 );
		backpatch( expression1->truelist, label1 );
		_false->set_res( expression1->res ); 
		expression1->falselist.push_back(_false);
		if( label2 != nullptr ) {
			backpatch( expression1->falselist, label2 );
		}
		append(Statement->next_vector,expression1->falselist);
		if ( statement1 != nullptr ) {
			append(Statement->break_vector,statement1->break_vector);
			append(Statement->continue_vector,statement1->continue_vector);
			append(Statement->case_vector,statement1->case_vector);
			append(Statement->next_vector,statement1->next_vector);
			append(Statement->return_vector,statement1->return_vector);
		}
		if ( _goto != nullptr ) {
			Statement->next_vector.push_back( _goto );
		}
    }
	switch_temp = nullptr;
    return Statement;
}


void create_switch( Expression * expression ){
	SymTabEntry * symbol = new SymTabEntry("switch:"+to_string(line_no), line_no, col_no  );
    symbol->type = Type(INT_T, 0, false);
	size_t size = symbol->type.get_size();
	local_symbol_table.offset = local_symbol_table.offset + WORD_SIZE;
	local_symbol_table.reqd_size = local_symbol_table.offset > local_symbol_table.reqd_size ? local_symbol_table.offset : local_symbol_table.reqd_size;
	Identifier * id = new Identifier(("switch:"+to_string(line_no)).c_str(), line_no, col_no);
	local_symbol_table.add_to_table( symbol, id, false);
	Address* addr1;
	MEMEMIT( expression, addr1 );
	Address* addr2 = new_3id(symbol);
	emit( addr2, "=", addr1, nullptr );
	expression->res = addr2;
}

Statement *create_selection_statement_switch(Expression *expression1, GoTo * _test, Statement* statement1, GoTo * _next ){
	SelectionStatement * Statement = new SelectionStatement();
	Statement->name = "SWITCH";
	Statement->add_children(expression1,statement1);

	Type t = expression1->type;
	if (!t.isInt()){
		error_message("Invalid Type " + t.get_name() + " in Switch Case", line_no);
	}

	Statement->next_vector.push_back(_next);

	Address *addr1, *addr2;
	MEMEMIT(expression1, addr1);
	addr2 = new_temp();
	backpatch(_test, create_new_label());
	Label * default_label = nullptr;
    for(auto i:switch_label){
		if (i.first == "" ) {
			default_label = i.second;
			continue;
		}
		Address * con = new_3const(0, INT3);
		con->string_name=i.first;
		emit( addr2 , "==", addr1, con );
		GoTo *_goto=create_new_goto_cond(addr2, true);
		backpatch(_goto, i.second);
	}
	if ( default_label != nullptr ) {
		create_new_goto(default_label);
	}
	switch_label.clear();
	switch_type=nullptr; 

	append(Statement->next_vector,statement1->break_vector);
	append(Statement->next_vector,statement1->next_vector);
	append(Statement->continue_vector,statement1->continue_vector);
	append(Statement->case_vector,statement1->case_vector);
	append(Statement->return_vector,statement1->return_vector);
	switch_temp = nullptr;
	return Statement;
}




//####################
// EXPRESSION STATEMENT 
//####################
Statement* create_expression_statement(Expression* expression1)
{
	if (expression1 == nullptr) {
		return nullptr;
	}
	ExpressionStatement *Statement = new ExpressionStatement();
	Statement->add_child(expression1);
	append(Statement->next_vector, expression1->truelist); 
	append(Statement->next_vector, expression1->falselist); 
	return Statement;
}


//####################
//ITERATION STATEMENT 
//####################
Statement* create_iteration_statement_for(Expression * expression1, Label * label1, Expression * expression2, Label * label2, Statement* statement1) {

	IterationStatement *Statement = new IterationStatement();
	Statement->name = "FOR";
	Statement->add_children( expression1, expression2, statement1);

	if (expression2 != nullptr) {
		backpatch( expression2->truelist, label2 );
		append(Statement->next_vector, expression2->falselist);
	}
	if (statement1 != nullptr) {
		backpatch(statement1->next_vector, label1);
		backpatch(statement1->continue_vector, label1);
		append(Statement->next_vector, statement1->break_vector);
		append(Statement->return_vector, statement1->return_vector);
	}
	GoTo * _goto = create_new_goto(label1);
	return Statement;
}


Statement* create_iteration_statement_for( Expression * expression1, Label * label1, Expression * expression2, GoTo * _goto1, Label * label2, Expression * expression3, GoTo * _goto2 , Label * label3, Statement* statement1 ){

	IterationStatement *Statement = new IterationStatement();
	Statement->name = "FOR";
	Statement->add_children(expression1, expression2, expression3, statement1);

	if (expression2 != nullptr){
		backpatch(expression2->truelist, label3);
		append(Statement->next_vector, expression2->falselist);
	}
	backpatch(_goto1, label3);
	backpatch(_goto2, label1);

	if (statement1 != nullptr) {
		backpatch(statement1->next_vector, label2);
		backpatch(statement1->continue_vector, label2);
		append(Statement->next_vector, statement1->break_vector);
		append(Statement->case_vector, statement1->case_vector);
		append(Statement->return_vector, statement1->return_vector);
	}
	GoTo * _goto = create_new_goto(label2);
	return Statement;
}


Statement* create_iteration_statement_do_while(Label *label1,Statement *statement1,Label *label2,Expression *expression1 , GoTo * _false)
{
	IterationStatement *Statement = new IterationStatement();
	Statement->add_children(expression1,statement1);
	Statement->name = "DO WHILE";
	
	backpatch(expression1->truelist,label1);
	_false->set_res(expression1->res); 
	expression1->falselist.push_back(_false);
	append(Statement->next_vector,expression1->falselist);
	if ( statement1 != nullptr ) {
		backpatch(statement1->next_vector, label2);
		backpatch(statement1->continue_vector, label2);
		append(Statement->next_vector, statement1->break_vector);
		append(Statement->return_vector, statement1->return_vector);
	}
	GoTo * _goto = create_new_goto(label1);
	return Statement;
}


Statement* create_iteration_statement_while(Label * label1, Expression *expression1, GoTo * _false, Label * label2, Statement *statement1 ){
	IterationStatement * Statement = new IterationStatement();
    Statement->name = "WHILE";
	Statement->add_children(expression1, statement1);

	backpatch(expression1->truelist,label2);
	_false->set_res( expression1->res ); 
	expression1->falselist.push_back(_false);
	append(Statement->next_vector,expression1->falselist);
	if ( statement1 != nullptr ) {
		backpatch(statement1->next_vector,label1);
		backpatch(statement1->continue_vector,label1);
		append(Statement->next_vector,statement1->break_vector);
		append(Statement->return_vector,statement1->return_vector);
	}
	GoTo * _goto = create_new_goto(label1);

	return Statement;
	
}


//####################
//JUMP STATEMENT 
//####################
Statement* create_jump_statement(int _type){
	JumpStatement *Statement = new JumpStatement();
	if (_type == BREAK){
		Statement->name = "break";
        GoTo * _goto = create_new_goto();
		Statement->break_vector.push_back(_goto);
	}
	if(_type == CONTINUE){
		Statement->name = "continue";
        GoTo * _goto = create_new_goto();
		Statement->continue_vector.push_back(_goto);
	}
	if (_type == RETURN){
		Statement->name = "return";
		create_new_return(nullptr);
		GoTo * _goto = create_new_goto();
		Statement->return_vector.push_back(_goto);
	}
	return Statement;
}

Statement* create_jump_statement_exp(Expression *expression){
	JumpStatement *Statement = new JumpStatement();
	Statement->name="return expression";
	Statement->add_child(expression);

	if (expression->type != invalid_type && local_symbol_table.return_type != expression->type) {
		error_message("Function expects return type of " + local_symbol_table.return_type.get_name() + ". Got " + expression->type.get_name(),line_no);
	}
	create_new_return(expression->res);
	GoTo * _goto = create_new_goto();
	Statement->return_vector.push_back(_goto);
	return Statement;
}

Statement* create_jump_statement_go_to(Identifier *id)
{
	JumpStatement *Statement = new JumpStatement();
	Statement->name = "GOTO";
	Statement->add_child(id);

	auto it = label_identifier.find(id->value);
	if(it != label_identifier.end()){
        GoTo * _goto = create_new_goto(it->second);
	}
	else {
		auto it = goto_identifier.find(id->value);
		if(it != goto_identifier.end()){
			vector<GoTo*>& vec = it->second;
			GoTo * _goto = create_new_goto();
			vec.push_back(_goto);
		}
		else{
			GoTo * _goto = create_new_goto();
			vector<GoTo*> & vec = * new vector<GoTo * >;
			vec.push_back(_goto);
			goto_identifier.insert({id->value, vec});
		}
	}
	return Statement;
}


//####################
//LABELLED STATEMENT 
//####################
Statement* create_labeled_statement_def(Label* label1, Statement* statement1){
	if(switch_temp!=NULL){
		backpatch(statement1->next_vector,switch_temp);
	}
	IterationStatement *Statement = new IterationStatement();
	Statement->name = "DEFAULT";
	Statement->add_child(statement1);

	switch_label.insert({"",label1});
	
	append(Statement->next_vector, statement1->break_vector);
	append(Statement->case_vector, statement1->case_vector);
	switch_temp = label1;
	append(Statement->break_vector, statement1->break_vector);
	append(Statement->next_vector,statement1->next_vector);
	append(Statement->case_vector, statement1->case_vector);
	append(Statement->return_vector,statement1->return_vector);
	return Statement;
} 

Statement* create_labeled_statement_iden( Identifier *id, Label* label, Statement* statement1 ){
	LabeledStatement *Statement = new LabeledStatement();
	Statement->name = "GOTO";
	Statement->add_children( id, statement1 );
    
	auto it = label_identifier.find( id->value );
	if (it == label_identifier.end()){
		label_identifier.insert({id->value, label });
	}
	else{
		error_message("duplicate label " + id->value, id->line_no, id->col_no);
		return Statement;
	}
	auto it2 = goto_identifier.find(id->value);
	if (it2 != goto_identifier.end()) {
		backpatch(it2->second,label);
		it2->second.clear();
		goto_identifier.erase(id->value);
	}
	return Statement;
}


Statement* create_labeled_statement_case(Constant *constant, Label* label1, Statement* statement1){
	IterationStatement *Statement = new IterationStatement();
	Statement->name = "CASE";
	Statement->add_children( constant ,statement1);
	
	if (! constant->ConstantType.isInt()) {
		error_message("Case label not an interger constant.",constant->line_no ,constant->col_no);
	}
	if (switch_type == NULL) {
		switch_type = &(constant->ConstantType);
	}
	string string_name = "";
	if ( constant->ConstantType.isUnsigned() && constant->ConstantType.isInt() ){
		string_name = to_string(constant->val.ui);
	}
	else if (constant->ConstantType.isInt()){
		string_name = to_string(constant->val.i);
	}
	auto it = switch_label.find(string_name);
	
	if ( it == switch_label.end() ){
		switch_label.insert({string_name,label1});
	}
	else{
		error_message("Duplicate case value",constant->line_no ,constant->col_no);
	}
	append(Statement->break_vector, statement1->break_vector);
	append(Statement->case_vector, statement1->case_vector);
	append(Statement->next_vector,statement1->next_vector);
	append(Statement->return_vector,statement1->return_vector);
	return Statement;
}




//######################
//STATEMENT LIST
//######################


Statement* create_statement_list( Statement * statement1 ){
	if ( statement1 == nullptr ) {
		return nullptr;
	}
	StatementList * Statement = new StatementList();
	Statement->name = "statement_list";
	Statement->add_child( statement1 );
	append(Statement->next_vector,statement1->next_vector);
	append(Statement->continue_vector,statement1->continue_vector);
	append(Statement->break_vector,statement1->break_vector);
	append(Statement->case_vector,statement1->case_vector);
	append(Statement->return_vector,statement1->return_vector);

	return Statement;

}

Statement* add_to_statement_list(Statement * statement1, Statement * statement2) {
	if (statement1 == nullptr) {
		return create_statement_list(statement2);
	}
	if (statement2 == nullptr) {
		return statement1;
	}
	
	statement1->add_child( statement2 );
	statement1->next_vector = statement2->next_vector;
	append(statement1->break_vector,statement2->break_vector);
	append(statement1->continue_vector,statement2->continue_vector);
	append(statement1->case_vector,statement2->case_vector);
	append(statement1->return_vector,statement2->return_vector);
	return statement1;

}


//########################
//COMPOUND STATEMENT
//########################

void backpatch_fn( Statement * statement ) {
	Label * label = create_new_label();
	backpatch(statement->return_vector,label);
	backpatch(statement->next_vector,label);
}
