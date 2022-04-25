#include <algorithm>
#include <assert.h>
#include <iostream>
#include <list>
#include <sstream>
#include <vector>
#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <iterator>

#include "ast.h"
#include "expression.h"
#include "statement.h"
#include "symtab.h"
#include "3ac.h"
#include "y.tab.h"

using namespace std;

Type* typeSwitch=nullptr;
Label* tmpSwitch=nullptr;
map<string, Label *> lblSwitch;
map<string,Label *> idLabel;
map<string,vector<GoTo *> & > idGoto;


Statement *create_selection_statement_if( Expression *ex, GoTo * _false, Label * l1, Statement *st1, GoTo * _goto, Label * l2, Statement *st2 ){
    SelectionStatement* CSSI = new SelectionStatement();
    if(st2==nullptr){
		CSSI->name = "IF";
        CSSI->add_children(ex,st1);
		_false->set_res( ex->res ); 
		backpatch( ex->truelist, l1 );
		ex->falselist.push_back(_false);
		if( l2 != nullptr ) {
			backpatch( ex->falselist, l2 );
		}
		append(CSSI->nextlist,ex->falselist);
		if ( st1 != nullptr ) {
			append(CSSI->nextlist,st1->nextlist);
			append(CSSI->breaklist,st1->breaklist);
			append(CSSI->continuelist,st1->continuelist);
			append(CSSI->caselist,st1->caselist);
			append(CSSI->returnlist,st1->returnlist);
		}
		if ( _goto != nullptr ) {
			CSSI->nextlist.push_back( _goto );
		}
    }
	else {
		assert(_goto != nullptr );
		assert(l2 != nullptr );
		CSSI->name = "IF ELSE";
    	CSSI->add_children(ex,st1,st2);
		_false->set_res( ex->res ); 
		backpatch( ex->truelist, l1 );
		ex->falselist.push_back(_false);
		backpatch( ex->falselist, l2 );
		if ( st1 != nullptr ) {
			append(CSSI->nextlist,st1->nextlist);
			append(CSSI->breaklist,st1->breaklist);
			append(CSSI->continuelist,st1->continuelist);
			append(CSSI->caselist,st1->caselist);
			append(CSSI->returnlist,st1->returnlist);
		}
		CSSI->nextlist.push_back( _goto );
		append(CSSI->nextlist,st2->nextlist);
		append(CSSI->breaklist,st2->breaklist);
		append(CSSI->continuelist,st2->continuelist);
		append(CSSI->caselist,st2->caselist);
		append(CSSI->returnlist,st2->returnlist);
    }
	tmpSwitch=nullptr;
    return CSSI;
}

void create_switch( Expression * ex ){
	Identifier * iden = new Identifier(("switch:"+to_string(line_no)).c_str(), line_no, col_no);
	SymTabEntry * symbol = new SymTabEntry("switch:"+to_string(line_no), line_no, col_no  );
	symbol->type = Type(INT_T, 0, false);
	local_symbol_table.offset += WORD_SIZE;
	if(local_symbol_table.offset > local_symbol_table.reqd_size){
		local_symbol_table.reqd_size = local_symbol_table.offset;
	}
	else{
		local_symbol_table.reqd_size = local_symbol_table.reqd_size;
	}
	local_symbol_table.add_to_table( symbol, iden, false);
	
	Address* t2 = new_3id(symbol);
	
	Address * t1;
	MEM_EMIT(ex,t1);
	
	emit(t2,"=",t1,nullptr);
	ex->res = t2;
}

Statement *create_selection_statement_switch(Expression *ex1, GoTo * _test, Statement* st1, GoTo * _next ){
	SelectionStatement * CSSS= new SelectionStatement();
	CSSS->name="SWITCH";
	CSSS->add_children(ex1,st1);
	Type t = ex1->type;
	if(t.isInt() == false){
		error_message("Invalid Type " + t.get_name() + " in Switch Case",line_no);
	}
	
	CSSS->nextlist.push_back(_next);
	Address *t1, *t2;
	MEM_EMIT(ex1,t1);
	t2= new_temp();
	backpatch(_test, create_new_label());
	Label * default_label = nullptr;
	for(auto i:lblSwitch){
		if (i.first == "" ) {
			default_label = i.second;
			continue;
		}
		Address * con = new_3const(0, INT3);
		con->name=i.first;
		emit( t2 , "==",t1,con );
		GoTo *_goto=create_new_goto_cond(t2,true);
		backpatch(_goto, i.second);
	}
	if ( default_label != nullptr ) {
		create_new_goto(default_label);
	}
	lblSwitch.clear();
	typeSwitch=nullptr; 
	append(CSSS->nextlist,st1->breaklist);
	append(CSSS->nextlist,st1->nextlist);
	append(CSSS->continuelist,st1->continuelist);
	append(CSSS->caselist,st1->caselist);
	append(CSSS->returnlist,st1->returnlist);
	tmpSwitch=nullptr;

	return CSSS;
}

Statement* create_expression_statement(Expression* ex1)
{
	if ( ex1 != nullptr ) {
		ExpressionStatement *CES= new ExpressionStatement();
		CES->add_child(ex1);
		append(CES->nextlist,ex1->truelist); 
		append(CES->nextlist,ex1->falselist); 
		return CES;		
	}
	return nullptr;
}

Statement* create_iteration_statement_while(Label * l1, Expression *ex1, GoTo * _false, Label * l2, Statement *st1 ){
	IterationStatement * CISW = new IterationStatement();
	CISW->name = "WHILE";
	CISW->add_children(ex1,st1);
	backpatch(ex1->truelist,l2);
	_false->set_res( ex1->res ); 
	ex1->falselist.push_back(_false);
	append(CISW->nextlist,ex1->falselist);
	if ( st1 != nullptr ) {
		backpatch(st1->nextlist,l1);
		backpatch(st1->continuelist,l1);
		append(CISW->nextlist,st1->breaklist);
		append(CISW->returnlist,st1->returnlist);
	}
	create_new_goto(l1);
	return CISW;
	
}

Statement* create_iteration_statement_do_while(Label *l1,Statement *st1,Label *l2,Expression *ex1 , GoTo * _false)
{
	IterationStatement *CIS=new IterationStatement();
	CIS->name="DO WHILE";
	CIS->add_children(ex1,st1);	
	backpatch(ex1->truelist,l1);
	_false->set_res( ex1->res ); 
	ex1->falselist.push_back(_false);
	append(CIS->nextlist,ex1->falselist);
	if ( st1 != nullptr ) {
		backpatch(st1->nextlist,l2);
		backpatch(st1->continuelist,l2);
		append(CIS->nextlist,st1->breaklist);
		append(CIS->returnlist,st1->returnlist);
	}
	create_new_goto(l1);
	return CIS;
}

Statement* create_iteration_statement_for(Expression * ex1, Label * l1, Expression * ex2, Label * l2, Statement* st1) {
	IterationStatement *CISF=new IterationStatement();
	CISF->add_children( ex1, ex2, st1);
	CISF->name = "FOR";
	if ( ex2 != nullptr ) {
		backpatch( ex2->truelist, l2 );
		append(CISF->nextlist, ex2->falselist);
	}
	if ( st1 != nullptr ) {
		backpatch(st1->nextlist,l1);
		backpatch(st1->continuelist,l1);
		append(CISF->nextlist, st1->breaklist);
		append(CISF->returnlist,st1->returnlist);
	}
	create_new_goto(l1);
	return CISF;
}


Statement* create_iteration_statement_for( Expression * ex1, Label * l1, Expression * ex2, GoTo * _goto1, Label * l2, Expression * ex3, GoTo * _goto2 , Label * l3, Statement* st1 ){

	IterationStatement *CISF=new IterationStatement();
	CISF->add_children( ex1, ex2, ex3, st1);
	CISF->name = "FOR";

	if ( ex2 != nullptr ) {
		backpatch( ex2->truelist, l3 );
		append(CISF->nextlist, ex2->falselist);
	}
	backpatch(_goto1, l3);
	backpatch(_goto2, l1);

	if ( st1 != nullptr ) {
		backpatch(st1->nextlist,l2);
		backpatch(st1->continuelist,l2);
		append(CISF->nextlist, st1->breaklist);
		append(CISF->caselist, st1->caselist);
		append(CISF->returnlist,st1->returnlist);
	}
	create_new_goto(l2);
	return CISF;
}

Statement* create_jump_statement_go_to(Identifier *id)
{
	JumpStatement *CJS=new JumpStatement();
	CJS->name = "GOTO";
	CJS->add_child(id);
	auto it = idLabel.find(id->value);
	if ( it != idLabel.end() ) {
		create_new_goto( it->second );
		
	}
	else {
		auto it = idGoto.find(id->value);
		if(it == idGoto.end()){
			GoTo * var_goto = create_new_goto();
			vector<GoTo * > & vec = * new vector<GoTo * >;
			vec.push_back(var_goto);
			idGoto.insert({id->value, vec });
		}
		else{
			vector<GoTo *> & vec = it->second;
			GoTo * var_goto = create_new_goto();
			vec.push_back(var_goto);
		}
	}
	return CJS;
}

Statement* create_jump_statement(int _type){
	JumpStatement *CJS=new JumpStatement();	
	if (_type == RETURN ) {
		CJS->name="return";
		create_new_return(nullptr);
		GoTo * _goto = create_new_goto();
		CJS->returnlist.push_back(_goto);
	}
	else if(_type == BREAK){
		GoTo * _goto = create_new_goto();
		CJS->name="break";
		CJS->breaklist.push_back(_goto);
	}
	else if (_type == CONTINUE){
		GoTo * _goto = create_new_goto();
		CJS->name="continue";
		CJS->continuelist.push_back(_goto);
	}
	return CJS;
}

Statement* create_jump_statement_exp(Expression *ex)
{	JumpStatement *CJSE=new JumpStatement();
	CJSE->add_child(ex);
	CJSE->name="return expression";
	if ( ex->type != invalid_type && local_symbol_table.return_type != ex->type ) {
		error_message("Function expects return type of " + local_symbol_table.return_type.get_name() + ". Got " + ex->type.get_name(),line_no);
	}
	create_new_return(ex->res);
	GoTo * var_goto = create_new_goto();
	CJSE->returnlist.push_back(var_goto);
	return CJSE;
}
Statement* create_labeled_statement_iden( Identifier *id, Label* l, Statement* st1 ){
	LabeledStatement *CLSI = new LabeledStatement();
	CLSI->add_children( id, st1 );
	CLSI->name = "GOTO";
	auto var = idLabel.find( id->value );
	if (var == idLabel.end()){
		idLabel.insert({id->value, l });
	}
	else{
		error_message("duplicate label " + id->value, id->line_no, id->col_no);
		return CLSI;
	}
	auto var2 = idGoto.find(id->value);
	if ( var2 != idGoto.end() ) {
		backpatch(var2->second,l);
		var2->second.clear();
		idGoto.erase(id->value);
	}
	return CLSI;
}


Statement* create_labeled_statement_case(Constant *con,Label* l,Statement* s1){
	IterationStatement *CLSC = new IterationStatement();
	CLSC->add_children( con ,s1);
	CLSC->name = "CASE";
	if (! con->ConstantType.isInt()) {
		error_message("Case label not an interger constant.",con->line_no,con->col_no);
		}
	if (typeSwitch==NULL)
		{typeSwitch=&(con->ConstantType);}
	string str="";
	
	if (con->ConstantType.isUnsigned() && con->ConstantType.isInt()){
		str=to_string(con->val.ui);
		}
	else if (con->ConstantType.isInt()){
		str=to_string(con->val.i);
		}
	auto var=lblSwitch.find(str);
	if (var==lblSwitch.end()){
		lblSwitch.insert({str,l});
	}
	else{		
		error_message("Duplicate case value",con->line_no,con->col_no);
	}
	append(CLSC->breaklist, s1->breaklist);
	append(CLSC->nextlist,s1->nextlist);
	append(CLSC->caselist, s1->caselist);
	append(CLSC->returnlist,s1->returnlist);
	return CLSC;
}

Statement* create_labeled_statement_def(Label* l, Statement* s1){
	if(tmpSwitch!=NULL)	backpatch(s1->nextlist,tmpSwitch);
	IterationStatement *CLSD = new IterationStatement();
	CLSD->name = "DEFAULT";
	CLSD->add_child(s1);
	lblSwitch.insert({"",l});
	append(CLSD->nextlist, s1->breaklist);
	append(CLSD->caselist, s1->caselist);
	tmpSwitch=l;
	append(CLSD->breaklist, s1->breaklist);
	append(CLSD->nextlist,s1->nextlist);
	append(CLSD->caselist, s1->caselist);
	append(CLSD->returnlist,s1->returnlist);
	return CLSD;
} 
Statement* create_statement_list( Statement * st1 ){
	if ( st1 == nullptr ) {
		return nullptr;
	}
	StatementList * CSL = new StatementList();
	CSL->name = "statement_list";
	CSL->add_child( st1 );
	append(CSL->nextlist,st1->nextlist);
	append(CSL->continuelist,st1->continuelist);
	append(CSL->breaklist,st1->breaklist);
	append(CSL->caselist,st1->caselist);
	append(CSL->returnlist,st1->returnlist);
	return CSL;
}
Statement* add_to_statement_list( Statement * stl, Statement * st1 ) {
	if ( st1 == nullptr ) return stl;
	else if ( stl == nullptr ) return create_statement_list( st1 );
	stl->nextlist = st1->nextlist;
	stl->add_child( st1 );
	append(stl->continuelist,st1->continuelist);
	append(stl->breaklist,st1->breaklist);
	append(stl->caselist,st1->caselist);
	append(stl->returnlist,st1->returnlist);
	return stl;
}
void backpatch_fn( Statement * s ) {
	Label * lbl = create_new_label();
	backpatch(s->returnlist,lbl);
	backpatch(s->nextlist,lbl);
}
