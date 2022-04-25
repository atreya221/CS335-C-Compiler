#include <bits/stdc++.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <assert.h>
#include <vector>
#include <unordered_set>

#include "ast.h"
#include "symtab.h"
#include "expression.h"
#include "3ac.h"


using namespace std;

unsigned long long labels = 1;
unsigned long long instructions = 1;

map< unsigned int, TacInfo > tac_info_table;
unordered_set<string> var_rep;
vector< ThreeAC * > ta_code;

Label::Label() : ThreeAC(false) {
	instruction_id = instructions;
	reference_count = 0;
	dead = false;
	name = "L"+to_string(labels++);
}

Label::~Label() {}
Label* create_new_label(){
	Label* l = new Label();
	ta_code.push_back(l);
	return l;
}
ostream& operator<<(ostream& os, const Label& l){
	os << l.name << ":" ;
	return os;
}
string Label::print() {
	stringstream ss;
	ss << *this;
	return ss.str();
}

Quad::Quad ( Address * _result, string _operation, Address * _arg1, Address * _arg2 ) { 

	operation = _operation;
	
	result.alive = true;
	result.addr = _result;
	result.next_use = nullptr;

	arg1.alive = true;
	arg1.addr = _arg1;
	arg1.next_use = nullptr;
	if( arg1.addr != nullptr ) {
		if( arg1.addr->ta_instr!=nullptr) {
			arg1.addr->ta_instr->dead = false;
		}
	}

	arg2.alive = true;
	arg2.addr = _arg2;
	arg2.next_use = nullptr;	
	if( arg2.addr != nullptr ) {
		if( arg2.addr->ta_instr != nullptr) {
			arg2.addr->ta_instr->dead = false;
		}
	}
	
	if ( operation == "()s" ) {
		dead = false;
		result.addr->ta_instr->dead = false;
	}
};

Quad::~Quad () {
	delete result.addr;
	delete arg1.addr;
	delete arg2.addr;
}

ostream& operator<<(ostream& os, const ADDRESS & a){
	os << *a.addr;
	if ( a.addr->type == CON ) {
		return os;
	}
	if ( a.alive ) os << "(L)";
	else os << "(D)";
	return os;
}

ostream& operator<<(ostream& os, const Quad& q){
	if ( q.arg2.addr != nullptr ) {
		os << q.instr << ": " << q.result << " = " << q.arg1 << " " << q.operation << " " << q.arg2 ;
	}else if ( q.operation == "=s" ) {
		os << q.instr << ": " <<  q.result  << " = " << q.arg1;
	}else if ( q.operation == "=" ) {
		os << q.instr << ": " <<  q.result  << " = " << q.arg1;
	}else if ( q.operation == "()s" ) {
		os << q.instr << ": " << "(" << q.result << ")" << " = " << q.arg1;
	}else if ( q.operation == "()" ) {
		os << q.instr << ": " << q.result << " = " <<  "(" << q.arg1 << ")" ;
	}else if ( q.operation == "la" ) {
		os << q.instr << ": " <<  q.result  << " la " << q.arg1;
	}else if ( q.operation == "push" ) {
		assert(0);
	}else if ( q.operation.substr(0,3) == "arg" ) {
		assert(0);
	}else {
		os << q.instr << ": " << q.result << " = " << q.operation << " " << q.arg1 ;
	}
	return os;
}

unsigned long long emit(  Address * result, string operation, Address * arg1, Address * arg2 ) {
	Quad * Q = new Quad(result,operation,arg1,arg2);
	if ( result != nullptr ) {
		result->ta_instr = Q;
	}
	if ( result == nullptr || result->type == ID3 ) {
		Q->dead = false;
	}

	ta_code.push_back(Q);
	return ta_code.size();

}

string Quad::print() {
	stringstream ss;
	ss << *this;
	return ss.str();
}

unsigned long long strings = 1;
unsigned long long temporaries = 1;

Address::Address(string _name, ADD_TYPE _type ) : name (_name) , size(0), type(_type), ta_instr(nullptr)  {};
Address::Address(long _value, ADD_TYPE _type ) : name (to_string(_value)) , size(0), type(_type), ta_instr(nullptr) {};

Address * new_temp() {
	Address * addr = new Address("t" + to_string(temporaries), TEMP );
	addr->size = WORD_SIZE;
	addr->table_id = TEMP_ID_MASK | temporaries;
	temporaries++;
	tac_info_table.insert({addr->table_id,TacInfo(false)});
	return addr;
}

Address * new_mem( Type & type ) {
	Address * addr = new Address("t" + to_string(temporaries), MEM );
	if( type.isChar() ) {
		addr->size = 1;
	} else {
		addr->size = WORD_SIZE;
	}
	addr->table_id = TEMP_ID_MASK | temporaries;
	temporaries++;
	tac_info_table.insert({addr->table_id,TacInfo(false)});
	return addr;
}

Address * new_3string( StringLiteral * sl ) {
	string string_name = "_str_" + to_string(strings);
	Address * addr = new Address(string_name, ID3);
	addr->size = WORD_SIZE;
	addr->table_id = STRING_MASK | strings++;
	tac_info_table.insert({addr->table_id,TacInfo()});
	main_mem_unit.memory_locations.insert({addr->table_id,create_memory_location( string_name, addr->table_id, 0, addr->size )});
	main_mem_unit.strings.insert({string_name,sl->value});
	set_is_ea( addr->table_id );
	return addr;
}

Address * new_3id(SymTabEntry * symbol) {
	Address * addr = new Address(symbol->name, ID3);
	if( symbol->type.isChar() ) {
		addr->size = 1;
	} else {
		addr->size = WORD_SIZE;
	}
	addr->table_id = symbol->id;
	tac_info_table.insert({addr->table_id,TacInfo(symbol)});
	main_mem_unit.memory_locations.insert({addr->table_id,create_memory_location( symbol->name, symbol->id, symbol->offset, addr->size )});
	if ( symbol->type.is_ea() ) {
		set_is_ea( symbol->id );
	}
	return addr;
}

ostream& operator<<(ostream& os, const Address& a){
	os << a.name;
	return os;
}
void backpatch(GoTo * _goto, Label* label){
	if ( label == nullptr ) return;
	_goto->label=label;
	label->reference_count++;
	return ;
}
void backpatch(vector<GoTo*> & go_v, Label* label){
	if ( label == nullptr ) return;
	auto itr = go_v.begin();
	while(itr != go_v.end()){
		(*itr)->label=label;
		(*itr)->label->reference_count++;
		itr++;
	}	
}

void append( vector <GoTo *> & v1, vector <GoTo *> & v2) {
	v1.insert(v1.end(), v2.begin(), v2.end());
}
unsigned long long get_next_instr() {
	return instructions++;
}

ThreeAC::ThreeAC() : instr ( get_next_instr() ), bb_no(0) , dead(true) { };
ThreeAC::ThreeAC(bool no_add ) : instr ( instructions ), dead(true) { };
ThreeAC::~ThreeAC () {};

GoTo::GoTo () : label(nullptr) , res({nullptr, true, nullptr}) { dead = false; };

GoTo::~GoTo () {
	delete label;
	delete res.addr;
}

GoTo * create_new_goto() {
	GoTo * Goto = new GoTo();
	ta_code.push_back(Goto);
	return Goto;
}

GoTo * create_new_goto( Label * label) {
	GoTo * Goto = new GoTo();
	Goto->label = label;
	label->reference_count++;
	ta_code.push_back(Goto);
	return Goto;
}

GoTo * create_new_goto_cond( Address * res, bool condition ) {
	GoTo * Goto = new GoTo();
	Goto->condition = condition;
	Goto->res.addr = res;
	if ( res != nullptr ) {
		if ( res->ta_instr != nullptr ) {
			res->ta_instr->dead = false;
		}
	}
	ta_code.push_back(Goto);
	return Goto;
}
		
string GoTo::print() {
	stringstream ss;
	ss << *this;
	return ss.str();
}
void GoTo::set_res( Address * _res ){
	res.addr = _res;
	if ( res.addr != nullptr ) {
		if ( res.addr->ta_instr != nullptr ){
			res.addr->ta_instr->dead = false;
		}
	}	
}

ostream& operator<<(ostream& os, const GoTo& g){
	os << g.instr <<": ";
	if ( g.res.addr != nullptr ) {
		if (g.condition != true ) {
			os << "br.false " << *(g.res.addr);
		} else {
			os << "br.true " << *(g.res.addr);
		}
		if ( g.res.addr->type != CON ) {
			if ( !g.res.alive ) {
				os << "(D)";
			} else {
				os << "(L)";
			}
		}
	} else {
		os << "goto";		
	} 
	if ( g.label == nullptr ) {
		os << " -----";
		return os;
	}
	os << " " << *(g.label);
	return os;
}

Arg::Arg( Address * _addr, int _num ) : ThreeAC(), arg({_addr, true, nullptr}), num(_num)  { 
	dead = false;
	if( arg.addr != nullptr ) {
		if ( arg.addr->ta_instr != nullptr ) {
			arg.addr->ta_instr->dead = false;
		}
	}

};

Arg * create_new_arg( Address * addr, int num ) {
	Arg * arg = new Arg(addr, num);
	ta_code.push_back(arg);
	return arg;
}

string Arg::print() {
	stringstream ss;
	ss << *this;
	return ss.str();
}

ostream& operator<<(ostream& os, const Arg& a){
	os << a.instr <<": ";
	if ( a.arg.addr != nullptr ) {
		os << "arg " << a.num << " = ";	
		os << *a.arg.addr;
		if ( !a.arg.alive ) { 
			os << "(D)"; 
		} else { 
			os << "(L)";
		}
	}
	return os;
}


Call::Call( Address * _addr, string f_name ) : ThreeAC(), retval({_addr, true, nullptr}), function_name( f_name)  {dead = false;};

Call * create_new_call( Address * addr, string f_name ) {
	Call * call = new Call(addr, f_name);
	ta_code.push_back(call);
	return call;
}

string Call::print() {
	stringstream ss;
	ss << *this;
	return ss.str();
}

ostream& operator<<(ostream& os, const Call& c){
	os << c.instr <<": ";
	if ( c.retval.addr != nullptr ) {
		os << *c.retval.addr;
		assert ( c.retval.addr->type != CON );
		if ( !c.retval.alive ) { 
			os << "(D)"; 
		} else { 
			os << "(L)";
		}
		os << " = ";
	}
	os << "call " << c.function_name; 
	return os;
}

Return::Return( Address * _retval ) : ThreeAC(), retval({_retval, true, nullptr}) {
	dead = false;
	if( retval.addr != nullptr ) {
		if ( retval.addr->ta_instr!=nullptr ) {
			retval.addr->ta_instr->dead = false;
		}
	}
}

Return::~Return() {};

string Return::print() {
	stringstream ss;
	ss << *this;
	return ss.str();
}

ostream& operator<<(ostream& os, const Return& r){
	os << r.instr << ": ";
	if ( r.retval.addr == nullptr ) {
		os << "return";
		return os;
	}
	os << "return " << *r.retval.addr;
	if ( r.retval.addr->type != CON ) {
		if ( !r.retval.alive ) {
			os << "(D)";
		} else {
			os << "(L)";
		}
	}
	return os;
}

Return * create_new_return( Address * retval ){
	Return * _return = new Return(retval);
	ta_code.push_back(_return);
	return _return;
}


TacInfo::TacInfo() : alive(true), next_use(nullptr), symbol(nullptr) {};
TacInfo::TacInfo(SymTabEntry * _symbol) : alive(true), next_use(nullptr), symbol(_symbol) {};
TacInfo::TacInfo( bool _alive ) : alive(_alive), next_use(nullptr), symbol(nullptr) {};

TacInfo * create_tac_info(SymTabEntry * symbol){
	TacInfo * tacInfo = new TacInfo();
	tacInfo->symbol = symbol;
	return tacInfo;
}


void arithmetic_optimise(Quad* q){
    string val1 = "";
    string val2 = "";

	if(q->arg1.addr==nullptr){
		if(q->arg1.addr!=nullptr && q->result.addr->name == q->arg1.addr->name && q->operation =="="){
			q->dead=true;
		}
		return;
	}
	if(q->arg2.addr==nullptr){
		if(q->arg1.addr!=nullptr && q->result.addr->name == q->arg1.addr->name && q->operation =="="){
			q->dead=true;
		}
		return;
	}


	if(q->arg1.addr != nullptr) val1= q->arg1.addr->name;
	if(q->arg2.addr != nullptr) val2 = q->arg2.addr->name;

	if(q->arg1.addr->type == CON && q->arg2.addr->type == CON){
		string oper = q->operation;
		int x = stoi(q->arg1.addr->name);
		int y = stoi(q->arg2.addr->name);
        
		if(oper=="+"){
			q->arg2.addr=nullptr;
			q->arg1.addr->name = to_string(x+y);
		}
		if(oper=="-"){
			q->arg2.addr=nullptr;
			q->arg1.addr->name=to_string(x-y);	
		}
		if(oper=="*"){
			q->arg2.addr=nullptr;
			q->arg1.addr->name=to_string(x*y);
		}
		if(oper=="/"){	
			if(y==0) return;
			else{
				q->arg1.addr->name = to_string(x/y);
				q->arg2.addr = nullptr;
			}
		}
		if(oper==">>"){
			q->arg1.addr->name=to_string(x>>y);
			q->arg2.addr=nullptr;
		}
		if(q->operation =="<<"){
			q->arg1.addr->name=to_string(x<<y);
			q->arg2.addr=nullptr;
		}
		if(oper=="|"){
			q->arg1.addr->name=to_string(x|y);
			q->arg2.addr=nullptr;
		}
		if(oper=="&"){
			q->arg1.addr->name=to_string(x&y);
			q->arg2.addr=nullptr;
		}
		q->operation="";
		if(q->result.addr->type==TEMP){
			q->dead=true;
			*q->result.addr = *q->arg1.addr;
		}
	}
	else{
		if((q->operation=="+" || q->operation=="-") && (q->arg2.addr->name=="0" || q->arg2.addr->name=="0.0")){
			q->operation="";			
			int size = q->result.addr->size;
			q->dead=true;
			q->arg2.addr=nullptr;
			*q->result.addr = *q->arg1.addr;
			q->result.addr->size = size;
		}
		else if(q->operation=="+" && (q->arg1.addr->name=="0" || q->arg1.addr->name=="0.0")){
			q->operation = "";
			int size = q->result.addr->size;
			q->dead=true;
			q->arg1=q->arg2;
			q->arg2.addr=nullptr;
			*q->result.addr = *q->arg1.addr;
			q->result.addr->size = size;
		}
		else if(q->operation=="-" && (q->arg1.addr->name=="0" || q->arg1.addr->name=="0.0")){
			;
		}
		else if(q->operation=="*" && (q->arg2.addr->name=="0" || q->arg2.addr->name=="0.0")){
			q->operation="";
			q->dead=true;
			q->arg1.addr->name="0";
			q->arg2.addr=nullptr;
			*q->result.addr = *q->arg1.addr;
		}
		else if(q->operation=="*" && (q->arg1.addr->name=="0" || q->arg1.addr->name=="0.0")){
			q->operation="";
			q->dead=true;
			q->arg1.addr->name="0";
			q->arg2.addr=nullptr;
			*q->result.addr = *q->arg1.addr;
		}
		else if(q->operation=="/" && (q->arg2.addr->name=="0" || q->arg2.addr->name=="0.0")){
			;
		}
		else if(q->operation=="/" && (q->arg1.addr->name=="0" || q->arg1.addr->name=="0.0")){
			q->operation="";
			q->dead=true;
			q->arg1.addr->name="0";
			q->arg2.addr=nullptr;
			*q->result.addr = *q->arg1.addr;
		}
		else if((q->operation=="*"|| q->operation=="/") && (q->arg2.addr->name=="1" || q->arg2.addr->name=="1.0")){
			q->operation="";
			q->dead=true;
			q->arg2.addr=nullptr;
			*q->result.addr = *q->arg1.addr;
		}
		else if(q->operation=="*" && (q->arg1.addr->name=="1" || q->arg1.addr->name=="1.0")){
			q->operation="";
			q->dead=true;
			q->arg1=q->arg2;
			q->arg2.addr=nullptr;
			*q->result.addr = *q->arg1.addr;
		}
		else if(q->operation=="*" && q->arg2.addr->type==CON && ceil(log2(stoi(q->arg2.addr->name))) == floor(log2(stoi(q->arg2.addr->name)))){
			q->operation="<<";
			q->arg2.addr->name=to_string((int)ceil(log2(stoi(q->arg2.addr->name))));
		}
		else if(q->operation=="*" && q->arg1.addr->type==CON && ceil(log2(stoi(q->arg1.addr->name))) == floor(log2(stoi(q->arg1.addr->name)))){
			q->operation="<<";
			q->arg1=q->arg2;
			q->arg2.addr->name=to_string((int)ceil(log2(stoi(q->arg1.addr->name))));
		}
		else if(q->operation=="/" && q->arg2.addr->type==CON && ceil(log2(stoi(q->arg2.addr->name))) == floor(log2(stoi(q->arg2.addr->name)))){
			q->operation=">>";
			q->arg2.addr->name=to_string((int)ceil(log2(stoi(q->arg2.addr->name))));
		}
		else if(q->operation=="/" && q->arg2.addr->name==q->arg1.addr->name){
			q->operation="";
			q->dead=true;
			q->arg1.addr->name="1";
			q->arg2.addr=nullptr;
			*q->result.addr = *q->arg1.addr;
		}
		else if(q->operation=="-" && q->arg2.addr->name==q->arg1.addr->name){
			q->operation="";
			q->dead=true;
			q->arg1.addr->name="0";
			q->arg2.addr=nullptr;
			*q->result.addr = *q->arg1.addr;
		}
	}
}

SaveLive::SaveLive() : ThreeAC(false) , save_temps(true) { dead = false;};


SaveLive * create_new_save_live(bool save_temps) {
	SaveLive *save = new SaveLive();
	save->save_temps = false;
	ta_code.push_back(save);
	return save;
}

SaveLive * create_new_save_live() {
	SaveLive *save = new SaveLive();
	ta_code.push_back(save);
	return save;
}

string SaveLive::print() {
	stringstream ss;
	ss << *this;
	return ss.str();
}

ostream& operator<<(ostream& os, const SaveLive& s){
	os << "Save Live";
	return os;
}


void optimise_pass1() {
	GoTo * goto1 = nullptr;
	GoTo * _goto2 = nullptr;
	Label * label1 = nullptr;
	Label * label2 = nullptr;
	Quad * quad = nullptr;
    
	for ( auto it = ta_code.begin(); it != ta_code.end(); it++ ){
		if ( (*it)->dead == true ) {
			continue;
		}
		goto1 = dynamic_cast<GoTo *>(*it); 
	
		if ( goto1 != nullptr && goto1->res.addr != nullptr && goto1->res.addr->ta_instr != nullptr ) {
			Quad * q = dynamic_cast<Quad *>(goto1->res.addr->ta_instr);
			if ( q != nullptr && q->operation == "!" ) {
				goto1->condition = !goto1->condition;
				*goto1->res.addr = *q->arg1.addr; 
				q->dead = true;
			}
		}



		if ( goto1 != nullptr && goto1->res.addr != nullptr && goto1->res.addr->type == CON) {
			int value = stoi(goto1->res.addr->name);
			if (( value == 0 && goto1->condition == false) || (value != 0 && goto1->condition == true )){
				;
			} else {
				goto1->label->reference_count--;
				goto1->dead = true;
				continue;
			}
			
			if( goto1->res.addr->ta_instr != nullptr ) {
				goto1->res.addr = nullptr;
				goto1->res.addr->ta_instr->dead = true;
			} else {
				delete goto1->res.addr;
				goto1->res.addr = nullptr;
			}

		}


		label1 = dynamic_cast<Label* >(*it);
		if( label1 != nullptr && label1->reference_count == 0 ) {
			(*it)->dead = true;
			continue;
		}
		if ( _goto2 != nullptr && _goto2->res.addr == nullptr && ( label1 == nullptr || label1->reference_count == 0)  ) {
			(*it)->dead = true;
			if ( goto1 != nullptr && goto1->label != nullptr ) {
				goto1->label->reference_count--;
			}
			continue;
		}

		if ( label2 != nullptr && label1 != nullptr ) {
			(*it)->dead = true;
			*label1 = *label2;
			label1->dead = true;
			continue;
		}

		if ( label2 != nullptr && goto1 != nullptr && goto1->res.addr == nullptr ) {
			label2->name = goto1->label->name;
			label2->instruction_id = goto1->label->instruction_id;
			goto1->label->reference_count += label2->reference_count;
			label2->dead = true;
		}

		quad = dynamic_cast<Quad *>(*it);
		if (quad!=nullptr){
			arithmetic_optimise(quad);
			;
			
		}
		_goto2 = goto1; 
		label2 = label1;
	}
}

void dump_and_reset_3ac( ) {
	optimise_pass1();
	create_basic_blocks();
	create_next_use_info();
	
	for ( auto it = ta_code.begin(); it != ta_code.end(); it++ ){
		tac_ss << "3AC: " << (*it)->bb_no << ":  " << (*it)->print();
		if( (*it)->dead == true ) {
			tac_ss << " xxxx";
		}
		tac_ss << "\n";
	}
	gen_asm_code( );
	instructions = 1;
	temporaries = 1;
	ta_code.clear();
	tac_info_table.clear();
}

void create_basic_blocks() {	
	unsigned int basic_blks = 1;
	bool incremented = true;
	for ( auto it = ta_code.begin(); it != ta_code.end(); it++ ){
		if ( (*it)->dead == true ) {
			continue;
		}
		Label * label = dynamic_cast<Label *> (*it);
		if ( label != nullptr && !incremented ) {
			basic_blks++;
			(*it)->bb_no = basic_blks;
			continue;
		}
		incremented = false;
		(*it)->bb_no = basic_blks;
		GoTo * _goto = dynamic_cast<GoTo *> (*it);
		Return * _return = dynamic_cast<Return *>(*it);
		if(   _goto != nullptr || _return != nullptr ) {
			basic_blks++;
			incremented = true;
		}
	}
}

void create_next_use_info(){
	auto it = ta_code.rbegin();
	unsigned int basic_blk = (*it)->bb_no;
	for ( ; it != ta_code.rend(); it++ ){
		if ( (*it)->dead == true ) continue;
		if ((*it)->bb_no != basic_blk ) {
			reset_tac_info_table();
			basic_blk = (*it)->bb_no;
		}

		Quad * quad = dynamic_cast<Quad *>(*it);

		if ( quad != nullptr ) {
			
			if ( quad->result.addr != nullptr ) {
				assert(quad->result.addr->type != CON);
				auto it = get_entry_from_table(quad->result.addr);
				quad->result.next_use = it->second.next_use;
				quad->result.alive = it->second.alive;
				if( quad->operation == "()s" ) {
					it->second.alive = true;
					it->second.next_use = &quad->result;
				} else {
					it->second.alive = false;
					it->second.next_use = nullptr;
				}

			}
			if ( quad->arg1.addr != nullptr && quad->arg1.addr->type != CON ) {
				auto it = get_entry_from_table(quad->arg1.addr);
				quad->arg1.alive = it->second.alive;
				quad->arg1.next_use = it->second.next_use;
				it->second.alive = true;
				it->second.next_use = &quad->arg1;
			}

			if ( quad->arg2.addr != nullptr && quad->arg2.addr->type != CON ) {
				auto it = get_entry_from_table(quad->arg2.addr);
				quad->arg2.alive = it->second.alive;
				quad->arg2.next_use = it->second.next_use;
				it->second.alive = true;
				it->second.next_use = &quad->arg2;
			}

			continue;	

		}

		GoTo * goto_ = dynamic_cast<GoTo *>(*it);

		if ( goto_ != nullptr && goto_->res.addr != nullptr && goto_->res.addr->type != CON ) {
			auto it = get_entry_from_table(goto_->res.addr);
			goto_->res.next_use = it->second.next_use;
			goto_->res.alive = it->second.alive;
			it->second.next_use = &goto_->res;
			it->second.alive = true;
			continue;

		}
		
		Return * return_ = dynamic_cast<Return *>(*it);

		if ( return_ != nullptr && return_->retval.addr != nullptr && return_->retval.addr->type != CON ) {
			auto it = get_entry_from_table(return_->retval.addr);
			return_->retval.next_use = it->second.next_use;
			if ( it->second.next_use == nullptr && !(return_->retval.addr->table_id && GLOBAL_SYM_MASK ) ) {
				return_->retval.alive = false;
			} else {
				return_->retval.alive = true;
			}
			it->second.alive = true;
			it->second.next_use = &return_->retval;
			continue;
		}


		Call *call = dynamic_cast<Call *>(*it);
		if ( call != nullptr && call->retval.addr != nullptr && call->retval.addr->type != CON ) {
			auto it = get_entry_from_table(call->retval.addr);
			call->retval.next_use = it->second.next_use;
			call->retval.alive = it->second.alive;
			it->second.next_use = nullptr;
			it->second.alive = false;
			continue;
		}

		Arg *arg = dynamic_cast<Arg *>(*it);
		if ( arg != nullptr && arg->arg.addr != nullptr && arg->arg.addr->type != CON ) {
			auto it = get_entry_from_table(arg->arg.addr);
			arg->arg.alive = it->second.alive;
			arg->arg.next_use = it->second.next_use;
			it->second.alive = true;
			it->second.next_use = &arg->arg;
			continue;
		}
	}
}

void reset_tac_info_table() {
    auto it = tac_info_table.begin();
	while (it != tac_info_table.end()) {
		if ( it->first & TEMP_ID_MASK ) {
			it->second.alive = false;
		} else {
			it->second.alive = true;
		}
		it->second.next_use = nullptr;
		it++;
	}
}

map <unsigned int, TacInfo >::iterator  get_entry_from_table( Address * a ) {
	auto it = tac_info_table.find(a->table_id);
	if ( it == tac_info_table.end()) assert(0);
	return it;
}
