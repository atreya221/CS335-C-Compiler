#ifndef __3AC_H__
#define __3AC_H__

#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <map>
#include <unordered_set>

#include "symtab.h"
#include "ast.h"

using namespace std;

unsigned long long get_next_instr();
extern stringstream tac_ss;


extern unsigned long long instr;
extern unsigned long long temps;
extern unsigned long long strings;


typedef enum ADD_TYPE_ {
	ID3 = 1,
	CON = 2,
	TEMP = 3,
	MEM = 4
} ADD_TYPE;


class ThreeAC;

#define TEMP_ID_MASK 0x80000000
#define STRING_MASK  0x08000000

class Address {
public:
	string string_name;
	int size;
	ADD_TYPE type;
	ThreeAC * ta_instr;
	unsigned int table_id;
	Address( long val, ADD_TYPE type);
	Address( string string_name, ADD_TYPE type);
};

typedef struct _addresses {
	Address * addr; 
	struct _addresses * next_addr;
	bool present;
} ADDRESS;

class TacInfo {
	private: 
		bool present; 
		ADDRESS * next_addr; 
		SymTabEntry * sym; 
	public:
		TacInfo();
		TacInfo(SymTabEntry * );
		TacInfo( bool );
	friend TacInfo * create_tac_info(SymTabEntry * sym);
	friend void reset_tac_info_table();
	friend void create_next_use_info();
};

TacInfo * create_tac_info(SymTabEntry * sym);
TacInfo * create_tac_info(SymTabEntry * sym, bool live, ThreeAC * next_addr);

extern map< unsigned int, TacInfo > tac_info_table;
map< unsigned int, TacInfo >::iterator get_entry_from_table( Address * );

ostream& operator<<(ostream& os, const Address& a);

extern unsigned long long instr;
Address * new_temp();
Address * new_mem( Type & t );


typedef enum _const_type {
	INT3 = 1,
	FLOAT3
} CONST_TYPE;

template <typename T>
Address * new_3const(T val, CONST_TYPE con ) {
	return new Address(to_string(val), CON);
}


Address * new_3id(SymTabEntry * sym);
Address * new_3string( StringLiteral * sl );

class ThreeAC {
public:
	unsigned int instr_no;
	unsigned int bb_no;
	bool dead;
	ThreeAC();
	ThreeAC( bool no_add );
	virtual ~ThreeAC();
	virtual string print() = 0;

};

extern vector< ThreeAC*> ta_code;


class Quad : public ThreeAC {
public:
	string operation;
	ADDRESS result, argument1, argument2;
	Quad ( Address * result, string op, Address * argument1, Address * argument2 );
	~Quad();
	string print();

};


ostream& operator<<(ostream& os, const ADDRESS & a);

#define BACKPATCH(a) { \
	Label * _lab; 			\
	if ( (a)->falselist.size() != 0 || (a)->truelist.size() != 0 ) { \
		_lab = create_new_label(); \
	} \
	backpatch((a)->falselist,_lab); \
	backpatch((a)->truelist,_lab); \
}

#define SAVEREGS( E, t1, t2 ) { \
    if ( (t1)->type == TEMP || (t1)->type == MEM ) { \
        (E)->res = (t1); \
	(E)->res->type = TEMP; \
    } else if ( (t2)->type == TEMP || (t2)->type == MEM ){ \
        (E)->res = (t2); \
	(E)->res->type = TEMP; \
    } else { \
        (E)->res = new_temp(); \
    } \
}

#define MEMEMIT(a,b) { \
	if ( (a)->res->type == MEM ) { \
		(b) = new_temp(); \
		emit((b),"()",(a)->res, nullptr); \
	} else if ( (a)->res->type == ID3 ) { \
		(b) = (a)->res; \
	} else { \
		(b) = (a)->res; \
	} \
}


unsigned long long emit(Address * result, string op, Address * argument1, Address * argument2);

ostream& operator<<(ostream& os, const Quad& q);


class Label : public ThreeAC {
	public:
		string string_name;
		unsigned long long instr_id;
		unsigned int ref_cnt;
        
		Label();
		~Label();
		string print();
};

Label* create_new_label();
extern unsigned long long labels;
 
class GoTo : public ThreeAC {
		Label * label;
	public : 
		ADDRESS res;
		bool cond;
		GoTo();
		~GoTo();
		string print();
		void set_res(Address *res);

        friend ostream& operator<<(ostream& os, const GoTo& _goto);
        friend GoTo * create_new_goto_cond(Address *res, bool condition);
        friend void optimise_pass1();
        friend void backpatch(vector<GoTo*> &go_v, Label* label);
        friend void backpatch(GoTo* _goto, Label* label);
        friend GoTo* create_new_goto(Label * label);
        friend void process_goto(GoTo* _goto);
};



GoTo * create_new_goto();
GoTo * create_new_goto(Label * label);
GoTo * create_new_goto_cond(Address * addr, bool condition);


ostream& operator<<(ostream& os, const Label& l);

ostream& operator<<(ostream& os, const GoTo& g);

class Arg : public ThreeAC {
	public:
		ADDRESS arg;
		int num;
		Arg( Address * addr, int cnt );
		string print();		
};

ostream& operator<<(ostream& os, const Arg& a );
Arg * create_new_arg( Address * addr , int cnt );

class Call : public ThreeAC {
	public:
		ADDRESS return_value;
		string function_name;
		
		Call( Address * addr, string str_name );
		string print();
		
};

ostream& operator<<(ostream& os, const Call& c);
Call * create_new_call( Address * addr , string str_name );

class Return : public ThreeAC {
	public : 
		string print();
		ADDRESS return_value;
		Return( Address * _return_value );
		~Return();
	friend Return * create_new_return( Address * return_value );
};

Return * create_new_return(Address * return_value);

ostream& operator<<(ostream& os, const Return& r);

class SaveLive : public ThreeAC {
	public:
		bool saved;
		SaveLive();
		string print();
};

ostream& operator<<(ostream& os, const SaveLive& s);
SaveLive * create_new_save_live(bool saved);
SaveLive * create_new_save_live();

extern unordered_set<string> vars;
void backpatch(GoTo* _goto, Label* label);
void backpatch( vector <GoTo * > & go_v, Label * label );

void append ( vector <GoTo *> & v1, vector <GoTo *> & v2);

void create_basic_blocks();
void reset_tac_info_table();
void create_next_use_info();
void optimise_pass1();
void dump_and_reset_3ac(); 
#endif
