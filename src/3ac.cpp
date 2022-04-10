#include <bits/stdc++.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <unordered_set>

#include "ast.h"
#include "expression.h"
#include "statement.h"
#include "symtab.h"
#include "3ac.h"

using namespace std;

unsigned long long labels = 1;
unsigned long long instructions = 1;

vector<ThreeAC*> ta_code;
unordered_set<string> var_rep;
map<unsigned int, TacInfo> tac_info_table;

Label::Label() : ThreeAC(false) {
	instr_id = instructions;
	ref_cnt = 0;
	string_name = "L"+to_string(labels++);
	dead = false;
}

Label::~Label() {}


string Label::print() {
	stringstream stream;
	stream << *this;
	return stream.str();
}


Label* create_new_label(){
	Label* label = new Label();
	ta_code.push_back(label);
	return label;
}


ostream& operator<<(ostream& os, const Label& label){
	os << label.string_name << ":" ;
	return os;
}
	

Quad::Quad(Address * addr, string op, Address * argument1, Address * argument2 ) { 

	this->operation = op;

	this->result.addr = addr;
	this->result.present = true;
	this->result.next_addr = nullptr;

	this->argument1.addr = argument1;
	this->argument1.present = true;
	this->argument1.next_addr = nullptr;

	this->argument2.addr = argument2;
	this->argument2.present = true;
	this->argument2.next_addr = nullptr;

	if ( this->operation == "()s" ) {
		this->dead = false;
		this->result.addr->ta_instr->dead = false;
	}
	if( this->argument1.addr != nullptr && this->argument1.addr->ta_instr!=nullptr) {
		this->argument1.addr->ta_instr->dead = false;
	}
	if( this->argument2.addr != nullptr && this->argument2.addr->ta_instr != nullptr) {
		this->argument2.addr->ta_instr->dead = false;
	}
};

Quad::~Quad () {
	delete result.addr;
	delete argument1.addr;
	delete argument2.addr;

}

ostream& operator<<(ostream& os, const ADDRESS &addr){
	os << *addr.addr;
	if (addr.addr->type == CON) {
		return os;
	 }

	if (!addr.present) {
		os << "(D)";
	} else {
		os << "(L)";
	}
	return os;
}

ostream& operator<<(ostream& os, const Quad& quad){
	if (quad.argument2.addr != nullptr) {
		os << quad.instr_no << ": " << quad.result << " = " << quad.argument1 << " " << quad.operation << " " << quad.argument2 ;
	}
    else if (quad.operation == "()") {
		os << quad.instr_no << ": " << quad.result << " = " <<  "(" << quad.argument1 << ")" ;
	}
    else if (quad.operation == "()s") {
		os << quad.instr_no << ": " << "(" << quad.result << ")" << " = " << quad.argument1;
	}
    else if (quad.operation == "=") {
		os << quad.instr_no << ": " <<  quad.result  << " = " << quad.argument1;
	}
    else if (quad.operation == "la") {
		os << quad.instr_no << ": " <<  quad.result  << " la " << quad.argument1;
	}
    else if (quad.operation == "=s") {
		os << quad.instr_no << ": " <<  quad.result  << " = " << quad.argument1;
	}
    else if (quad.operation == "push") {
		assert(0);
	}
    else if (quad.operation.substr(0,3) == "arg") {
		assert(0);
	} 
    else {
		os << quad.instr_no << ": " << quad.result << " = " << quad.operation << " " << quad.argument1 ;
	}
	return os;
}

unsigned long long emit(Address *addr, string op, Address * argument1, Address * argument2) {
	Quad * quad = new Quad(addr, op, argument1, argument2);
	if (addr == nullptr || addr->type == ID3 ) {
		quad->dead = false;
	}
	if (addr != nullptr) {
		addr->ta_instr = quad;
	}
	ta_code.push_back(quad);
	return ta_code.size();

}

string Quad::print() {
	stringstream stream;
	stream << *this;
	return stream.str();
}

unsigned long long strings = 1;
unsigned long long temporaries = 1;

Address::Address(string string_name, ADD_TYPE type ) : string_name (string_name) , size(0), type(type), ta_instr(nullptr)  {};
Address::Address(long val, ADD_TYPE type ) : string_name (to_string(val)) , size(0), type(type), ta_instr(nullptr) {};

Address *new_temp() {
	Address * addr = new Address("t" + to_string(temporaries), TEMP );
	addr->size = WORD_SIZE;
	addr->table_id = TEMP_ID_MASK | temporaries;
	temporaries++;
	tac_info_table.insert({addr->table_id, TacInfo(false)});
	return addr;
}

Address * new_mem(Type & type) {
	Address * addr = new Address("t" + to_string(temporaries), MEM);
	addr->size = !type.isChar() ? WORD_SIZE : 1;
	addr->table_id = TEMP_ID_MASK | temporaries;
	temporaries++;
	tac_info_table.insert({addr->table_id, TacInfo(false)});
	return addr;
}

Address * new_3id(SymTabEntry * symbol) {
	Address * a = new Address(symbol->name, ID3);
	a->table_id = symbol->id;
	a->size = symbol->type.isChar() ? 1 : WORD_SIZE;
	tac_info_table.insert({a->table_id,TacInfo(symbol)});
	// mmu.memory_locations.insert({a->table_id,create_memory_location( symbol->name, symbol->id, symbol->offset, a->size )});
	if ( symbol->type.is_ea() ) {
		// set_is_ea( symbol->id );
	}
	return a;
}

Address * new_3string( StringLiteral * sl ) {
	string name = "_str_" + to_string(strings);
	Address * a = new Address(name, ID3);
	a->table_id = STRING_MASK | strings++;
	a->size = WORD_SIZE;
	tac_info_table.insert({a->table_id,TacInfo()});
	// mmu.memory_locations.insert({a->table_id,create_memory_location( name, a->table_id, 0, a->size )});
	// mmu.strings.insert({name,sl->value});
	// set_is_ea( a->table_id );
	return a;
}

ostream& operator<<(ostream& os, const Address& addr){
	if ( addr.type == ID3 ) {
		os << addr.string_name;
	} else {
		os << addr.string_name;
	}
	return os;
}
void backpatch(vector<GoTo*> & arg, Label* label){
	if ( label == nullptr ) return;
	for ( auto it = arg.begin(); it != arg.end(); it++ ) {
		label->ref_cnt++;
		(*it)->label=label;
	}
	return ;
}

void backpatch(GoTo * arg, Label* label){
	if ( label == nullptr ) {
		return;
	}
	label->ref_cnt++;
	arg->label=label;
	return ;
}

void append( vector <GoTo *> & vec1, vector <GoTo *> & vec2) {
	vec1.insert(vec1.end(), vec2.begin(), vec2.end());
}


ThreeAC::ThreeAC() : instr_no ( get_next_instr() ), bb_no(0) , dead(true) { };
ThreeAC::ThreeAC(bool no_add ) : instr_no ( instructions ), dead(true) { };
ThreeAC::~ThreeAC () {};

unsigned long long get_next_instr() {return instructions++;}

GoTo::GoTo () : label(nullptr) , res({nullptr, nullptr, true}) { dead = false; };

GoTo::~GoTo () {
	delete res.addr;
	delete label;
}

GoTo * create_new_goto() {
	GoTo * arg = new GoTo();
	ta_code.push_back(arg);
	return arg;
}

GoTo * create_new_goto( Label * label) {
	GoTo * arg = new GoTo();
	arg->label = label;
	label->ref_cnt++;
	ta_code.push_back(arg);
	return arg;
}

GoTo * create_new_goto_cond( Address * res, bool conditional ) {
	GoTo * arg = new GoTo();
	arg->cond = conditional;
	arg->res.addr = res;
	if ( res->ta_instr != nullptr ){
		if( res != nullptr ) 
			res->ta_instr->dead = false;
	}
	ta_code.push_back(arg);
	return arg;
}
		
void GoTo::set_res( Address * _addr ){
	res.addr = _addr;
	if ( _addr != nullptr )
		if( res.addr->ta_instr != nullptr ) 
			res.addr->ta_instr->dead = false;
}

string GoTo::print() {
	stringstream ss;
	ss << *this;
	return ss.str();
}

ostream& operator<<(ostream& os, const GoTo& goto_arg){
	os << goto_arg.instr_no <<": ";
	if ( goto_arg.res.addr == nullptr ) os << "goto"; 
	else {
		if (goto_arg.cond == false ) os << "br.false " << *(goto_arg.res.addr);		
		else os << "br.true " << *(goto_arg.res.addr);
		if ( goto_arg.res.addr->type != CON &&  goto_arg.res.present == false) {os << "(D)";}
		if ( goto_arg.res.addr->type != CON &&  goto_arg.res.present == true) {os << "(L)";}
	} 
	if ( goto_arg.label == nullptr ) {
		os << " -----";
		return os;
	}
	os << " " << *(goto_arg.label);
	return os;
}

Arg::Arg( Address * _addr, int _num ) : ThreeAC(), arg({_addr, nullptr, true}), num(_num) { 
	dead = false;
	if( arg.addr->ta_instr != nullptr ) {
		if( arg.addr != nullptr ){
			arg.addr->ta_instr->dead = false;
		}
	}
};

Arg * create_new_arg( Address * addr, int val ) {
	Arg * arg1 = new Arg(addr, val);
	ta_code.push_back(arg1);
	return arg1;
}

string Arg::print() {
	stringstream ss;
	ss << *this;
	return ss.str();
}

ostream& operator<<(ostream& os, const Arg& arg1){
	os << arg1.instr_no <<": ";
	if ( arg1.arg.addr != nullptr ) {
		os << "arg " << arg1.num << " = ";	
		os << *arg1.arg.addr;
		if ( arg1.arg.present == false ) { os << "(D)"; }			
		else { os << "(L)"; }
	}
	return os;
}


Call::Call( Address * addr, string str_name ) : ThreeAC(), return_value({addr, nullptr, true}), function_name( str_name)  {
	dead = false;
};

Call * create_new_call( Address * addr, string str_name ) {
	Call * c = new Call(addr, str_name);
	ta_code.push_back(c);
	return c;
}

string Call::print() {
	stringstream ss;
	ss << *this;
	return ss.str();
}


ostream& operator<<(ostream& os, const Call& call_arg){
	os << call_arg.instr_no <<": ";
	if ( call_arg.return_value.addr != nullptr ) {
		os << *call_arg.return_value.addr;
		assert ( call_arg.return_value.addr->type != CON );		
		if ( call_arg.return_value.present == false ) os << "(D)";
		else os << "(L)"; 
		os << " = ";
	}
	os << "call " << call_arg.function_name; 
	return os;
}


Return::Return( Address * ret_val ) : ThreeAC(), return_value({ret_val, nullptr, true}) {
	dead = false;
	if( return_value.addr != nullptr )
		if (return_value.addr->ta_instr!=nullptr) {
		return_value.addr->ta_instr->dead = false;
	}
}

Return::~Return() {};

string Return::print() {
	stringstream ss;
	ss << *this;
	return ss.str();
}

ostream& operator<<(ostream& os, const Return& r){
	os << r.instr_no << ": ";
	if ( r.return_value.addr == nullptr ) {
		os << "return";
		return os;
	}
	os << "return " << *r.return_value.addr;
	if ( r.return_value.addr->type != CON && r.return_value.present ) { os << "(L)"; }
	if ( r.return_value.addr->type != CON && !r.return_value.present ) { os << "(D)"; }
	
	return os;
}

Return * create_new_return( Address * return_value ){
	Return * ret = new Return(return_value);
	ta_code.push_back(ret);
	return ret;
}


TacInfo::TacInfo() : present(true), next_addr(nullptr), sym(nullptr) {};
TacInfo::TacInfo(SymTabEntry * sym) : present(true), next_addr(nullptr), sym(sym) {};
TacInfo::TacInfo( bool is_present ) : present(is_present), next_addr(nullptr), sym(nullptr) {};



TacInfo * create_tac_info(SymTabEntry * sym){
	TacInfo * tacInfo = new TacInfo();
	tacInfo->sym = sym;
	return tacInfo;
}

void arithmetic_optimise(Quad* quad){
    string str1 = "";
    string str2 = "";
	if(quad->argument1.addr == nullptr || quad->argument2.addr == nullptr){
		if( quad->argument1.addr!=nullptr ){
			if( quad->result.addr->string_name == quad->argument1.addr->string_name ){
				if( quad->operation =="=" ){
						quad->dead=true;
				}
			}
		}
		return;
	}


	if(quad->argument1.addr!=nullptr){
		str1 = quad->argument1.addr->string_name;
	}

	if(quad->argument2.addr!=nullptr){
		str2 = quad->argument2.addr->string_name;
	}
	

	if(quad->argument2.addr->type == CON && quad->argument1.addr->type == CON){
		
		int num1 = stoi(quad->argument1.addr->string_name);
		string op = quad->operation;
		int num2 = stoi(quad->argument2.addr->string_name);
		if(op=="-"){
			quad->argument1.addr->string_name=to_string( num1 - num2 );
			quad->argument2.addr = nullptr;	
		}
		else if(op == "+"){
			quad->argument2.addr = nullptr;
			quad->argument1.addr->string_name = to_string( num1 + num2 );
		}
		else if(op == "/"){
			if (num2 != 0 ){
				quad->argument1.addr->string_name = to_string( num1 / num2 );
				quad->argument2.addr=nullptr;
			}
			else{
				return;
			}
		}
		else if(op == "*"){
			quad->argument1.addr->string_name = to_string( num1 * num2 );
			quad->argument2.addr = nullptr;
		}
		else if(quad->operation == "<<"){
			quad->argument1.addr->string_name = to_string( num1<<num2 );
			quad->argument2.addr = nullptr;
		}
		else if(op == ">>"){
			quad->argument1.addr->string_name = to_string( num1>>num2 );
			quad->argument2.addr = nullptr;
		}
		else if(op == "&"){
			quad->argument1.addr->string_name = to_string( num1&num2 );
			quad->argument2.addr = nullptr;
		}
		else if(op == "|"){
			quad->argument1.addr->string_name = to_string( num1|num2 );
			quad->argument2.addr = nullptr;
		}
		quad->operation = "";
		if(quad->result.addr->type == TEMP){
			*quad->result.addr = *quad->argument1.addr;
			quad->dead=true;
		}
	}
	else{

		if((quad->argument2.addr->string_name == "0.0" || quad->argument2.addr->string_name == "0") && (quad->operation == "+" || quad->operation == "-")){
			int size = quad->result.addr->size;
			quad->dead = true;
			quad->operation = "";		
			quad->argument2.addr = nullptr;
			*quad->result.addr = *quad->argument1.addr;
			quad->result.addr->size = size;
		}
		else if((quad->argument1.addr->string_name == "0.0" || quad->argument1.addr->string_name == "0") && quad->operation == "+" ){
			int size = quad->result.addr->size;
			quad->dead = true;
			quad->operation = "";
			quad->argument1 = quad->argument2;
			quad->argument2.addr = nullptr;
			*quad->result.addr = *quad->argument1.addr;
			quad->result.addr->size = size;
		}
		else if((quad->argument1.addr->string_name=="0.0" || quad->argument1.addr->string_name=="0") && quad->operation=="-"){
		}
		else if((quad->argument2.addr->string_name == "0.0" || quad->argument2.addr->string_name == "0") && quad->operation == "*"){
			quad->dead = true;
			quad->operation = "";
			quad->argument1.addr->string_name = "0";
			quad->argument2.addr = nullptr;
			*quad->result.addr = *quad->argument1.addr;
		}
		else if((quad->argument1.addr->string_name=="0.0" || quad->argument1.addr->string_name=="0") && quad->operation=="*"){
			quad->dead = true;
			quad->operation = "";
			quad->argument1.addr->string_name = "0";
			quad->argument2.addr = nullptr;
			*quad->result.addr = *quad->argument1.addr;
		}
		else if((quad->argument2.addr->string_name=="0.0" || quad->argument2.addr->string_name=="0") && quad->operation=="/"){
		}
		else if((quad->argument1.addr->string_name=="0.0" || quad->argument1.addr->string_name=="0") && quad->operation=="/"){
			quad->dead = true;
			quad->operation = "";
			quad->argument1.addr->string_name = "0";
			quad->argument2.addr = nullptr;
			*quad->result.addr = *quad->argument1.addr;
		}
		else if((quad->argument2.addr->string_name=="1.0" || quad->argument2.addr->string_name=="1") && (quad->operation=="*" || quad->operation=="/")){
			quad->dead = true;
			quad->operation = "";
			quad->argument2.addr = nullptr;
			*quad->result.addr = *quad->argument1.addr;
		}
		else if((quad->argument1.addr->string_name=="1.0" || quad->argument1.addr->string_name=="1") && quad->operation=="*"){
			quad->dead=true;
			quad->operation="";
			quad->argument1=quad->argument2;
			quad->argument2.addr=nullptr;
			*quad->result.addr = *quad->argument1.addr;
		}
		else if(quad->argument2.addr->type == CON && ceil(log2(stoi(quad->argument2.addr->string_name))) == floor(log2(stoi(quad->argument2.addr->string_name))) && quad->operation == "*"){
			quad->argument2.addr->string_name = to_string((int)ceil(log2(stoi(quad->argument2.addr->string_name))));
			quad->operation = "<<";
		}
		else if(quad->argument1.addr->type == CON && ceil(log2(stoi(quad->argument1.addr->string_name))) == floor(log2(stoi(quad->argument1.addr->string_name))) && quad->operation == "*"){
			quad->operation = "<<";
			quad->argument1 = quad->argument2;
			quad->argument2.addr->string_name = to_string((int)ceil(log2(stoi(quad->argument1.addr->string_name))));
		}
		else if(quad->argument2.addr->type == CON && ceil(log2(stoi(quad->argument2.addr->string_name))) == floor(log2(stoi(quad->argument2.addr->string_name))) && quad->operation == "/"){
			quad->argument2.addr->string_name = to_string((int)ceil(log2(stoi(quad->argument2.addr->string_name))));
			quad->operation = ">>";
		}
		else if(quad->argument2.addr->string_name == quad->argument1.addr->string_name && quad->operation == "-"){
			quad->dead = true;
			quad->operation = "";
			quad->argument1.addr->string_name = "0";
			quad->argument2.addr = nullptr;
			*quad->result.addr = *quad->argument1.addr;
		}
		else if(quad->argument2.addr->string_name == quad->argument1.addr->string_name && quad->operation == "/"){
			quad->dead = true;
			quad->operation = "";
			quad->argument1.addr->string_name = "1";
			quad->argument2.addr = nullptr;
			*quad->result.addr = *quad->argument1.addr;
		}
	}
}


SaveLive::SaveLive() : ThreeAC(false), saved(true) { this->dead = false;};


SaveLive * create_new_save_live() {
	SaveLive *save = new SaveLive();
	ta_code.push_back(save);
	return save;
}
SaveLive * create_new_save_live(bool saved) {
	SaveLive *save = new SaveLive();
	save->saved = false;
	ta_code.push_back(save);
	return save;
}

string SaveLive::print() {
	stringstream stream;
	stream << *this;
	return stream.str();
}

ostream& operator<<(ostream& os, const SaveLive& save){
	os << "Save Live";
	return os;
}



void optimise_pass1() {
	GoTo * argument1 = nullptr;
	GoTo * argument2 = nullptr;
	Label * label1 = nullptr;
	Label * label2 = nullptr;
	Quad * quad = nullptr;

	for ( auto it = ta_code.begin(); it != ta_code.end(); it++ ){
		if ( (*it)->dead == true ) {
			continue;
		}
		argument1 = dynamic_cast<GoTo *>(*it); 
	
		if ( argument1 != nullptr && argument1->res.addr != nullptr && argument1->res.addr->ta_instr != nullptr ) {
			Quad * q = dynamic_cast<Quad *>(argument1->res.addr->ta_instr);
			if ( q != nullptr && q->operation == "!" ) {
				argument1->cond = !argument1->cond;
				*argument1->res.addr = *q->argument1.addr; 
				q->dead = true;
			}
		}



		if ( argument1 != nullptr && argument1->res.addr != nullptr && argument1->res.addr->type == CON) {
			int value = stoi(argument1->res.addr->string_name);
			if (( value == 0 && argument1->cond == false) || (value != 0 && argument1->cond == true )){
			} else {
				argument1->label->ref_cnt--;
				argument1->dead = true;
				continue;
			}
			
			if( argument1->res.addr->ta_instr != nullptr ) {
				argument1->res.addr = nullptr;
				argument1->res.addr->ta_instr->dead = true;
			} else {
				delete argument1->res.addr;
				argument1->res.addr = nullptr;
			}

		}


		label1 = dynamic_cast<Label* >(*it);
		if( label1 != nullptr && label1->ref_cnt == 0 ) {
			(*it)->dead = true;
			continue;
		}
		if ( argument2 != nullptr && argument2->res.addr == nullptr && ( label1 == nullptr || label1->ref_cnt == 0)  ) {
			(*it)->dead = true;
			if ( argument1 != nullptr && argument1->label != nullptr ) {
				argument1->label->ref_cnt--;
			}
			continue;
		}

		if ( label2 != nullptr && label1 != nullptr ) {
			(*it)->dead = true;
			*label1 = *label2;
			label1->dead = true;
			continue;
		}

		if ( label2 != nullptr && argument1 != nullptr && argument1->res.addr == nullptr ) {
			label2->string_name = argument1->label->string_name;
			label2->instr_id = argument1->label->instr_id;
			label2->dead = true;
			argument1->label->ref_cnt += label2->ref_cnt;
		}

		quad = dynamic_cast<Quad *>(*it);
		if (quad!=nullptr) arithmetic_optimise(quad);
		argument2 = argument1; 
		label2 = label1;
	}
}

void dump_and_reset_3ac( ) {
	optimise_pass1();
	create_basic_blocks();
	create_next_use_info();
	
	for (auto it = ta_code.begin(); it != ta_code.end(); it++){
		tac_ss << "3AC: " << (*it)->bb_no << ":  " << (*it)->print();
		if( (*it)->dead == true ) {
			tac_ss << " xxxx";
		}
		tac_ss << "\n";
	}
	// gen_asm_code( );
    
	ta_code.clear();
	tac_info_table.clear();

	instructions = 1;
	temporaries = 1;
}

void create_basic_blocks() {	
	bool incremented = true;
	unsigned int basic_blocks = 1;
	for ( auto it = ta_code.begin(); it != ta_code.end(); it++ ){
		if ( (*it)->dead == false ) {
            Label * label = dynamic_cast<Label *> (*it);
            if (!incremented && label != nullptr) {
                basic_blocks++;
                (*it)->bb_no = basic_blocks;
                continue;
            }
            (*it)->bb_no = basic_blocks;
            Call * call = dynamic_cast<Call *>(*it);
            GoTo * argument = dynamic_cast<GoTo *> (*it);
            Return * return_ = dynamic_cast<Return *>(*it);
            incremented = false;
            if(argument != nullptr || return_ != nullptr) {
                incremented = true;
                basic_blocks++;
            }
		}
	}
}

void create_next_use_info(){
	auto it = ta_code.rbegin();
	unsigned int basic_blk = (*it)->bb_no;
	for (; it != ta_code.rend(); it++ ){
		if ( (*it)->dead == true ) {
			continue;
		}

		if ((*it)->bb_no != basic_blk ) {
			reset_tac_info_table();
			basic_blk = (*it)->bb_no;
		}

		Quad * quad = dynamic_cast<Quad *>(*it);

		if (quad != nullptr) {
			if (quad->result.addr != nullptr) {
				assert(quad->result.addr->type != CON);
				auto it = get_entry_from_table(quad->result.addr);
				quad->result.next_addr = it->second.next_addr;
				quad->result.present = it->second.present;
				if( quad->operation == "()s" ) {
					it->second.next_addr = &quad->result;
					it->second.present = true;
				} else {
					it->second.next_addr = nullptr;
					it->second.present = false;
				}

			}
			if ( quad->argument2.addr != nullptr && quad->argument2.addr->type != CON ) {
				auto it = get_entry_from_table(quad->argument2.addr);
				quad->argument2.next_addr = it->second.next_addr;
				quad->argument2.present = it->second.present;
				it->second.next_addr = &quad->argument2;
				it->second.present = true;
			}
			if (quad->argument1.addr != nullptr && quad->argument1.addr->type != CON) {
				auto it = get_entry_from_table(quad->argument1.addr);
				quad->argument1.next_addr = it->second.next_addr;
				quad->argument1.present = it->second.present;
				it->second.next_addr = &quad->argument1;
				it->second.present = true;
			}

			continue;	

		}

		GoTo * goto_ = dynamic_cast<GoTo *>(*it);

		if ( goto_ != nullptr && goto_->res.addr != nullptr && goto_->res.addr->type != CON ) {
			auto it = get_entry_from_table(goto_->res.addr);
			goto_->res.next_addr = it->second.next_addr;
			goto_->res.present = it->second.present;
			it->second.next_addr = &goto_->res;
			it->second.present = true;
			continue;
		}
		
		Return * return_ = dynamic_cast<Return *>(*it);
		if ( return_ != nullptr && return_->return_value.addr != nullptr && return_->return_value.addr->type != CON ) {
			auto it = get_entry_from_table(return_->return_value.addr);
			return_->return_value.next_addr = it->second.next_addr;
			if ( it->second.next_addr == nullptr && !(return_->return_value.addr->table_id && GLOBAL_SYMBOL_MASK ) ) {
				return_->return_value.present = false;
			} else {
				return_->return_value.present = true;
			}
			it->second.next_addr = &return_->return_value;
			it->second.present = true;
			continue;
		}


		Call * call_ = dynamic_cast<Call *>(*it);
		if ( call_ != nullptr && call_->return_value.addr != nullptr && call_->return_value.addr->type != CON ) {
			auto it = get_entry_from_table(call_->return_value.addr);
			call_->return_value.next_addr = it->second.next_addr;
			call_->return_value.present = it->second.present;
			it->second.next_addr = nullptr;
			it->second.present = false;
			continue;
		}

		Arg * argument = dynamic_cast<Arg *>(*it);
		if ( argument != nullptr && argument->arg.addr != nullptr && argument->arg.addr->type != CON ) {
			auto it = get_entry_from_table(argument->arg.addr);
			argument->arg.next_addr = it->second.next_addr;
			argument->arg.present = it->second.present;
			it->second.next_addr = &argument->arg;
			it->second.present = true;
			continue;
		}
	}
}

void reset_tac_info_table() {

	for ( auto it = tac_info_table.begin(); it != tac_info_table.end(); it++ ) {
		if (TEMP_ID_MASK & it->first) {
			it->second.present = false;
		} else {
			it->second.present = true;
		}
		it->second.next_addr = nullptr;
		
	}
}

map <unsigned int, TacInfo >::iterator get_entry_from_table(Address * addr) {
	if (tac_info_table.find(addr->table_id) == tac_info_table.end()) assert(0);
	return tac_info_table.find(addr->table_id);
}
