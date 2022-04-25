#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include <sstream>
#include <iostream>
#include <vector>
#include <set>
#include "3ac.h"


#define NUM_ARCH_REGS 20
#define NUM_TEMP_REGS 10
#define NUM_REG_ARGS 4

using namespace std;

extern stringstream asm_ss;

class Address;
typedef struct _addresses ADDRESS;


typedef enum ARCH_REG_ {
	tINV = -1,
	zz = 0,
	at = 1,
	v0 = 2,
	v1 = 3,
	a0 = 4,
	a1 = 5,
	a2 = 6,
	a3 = 7,
	t0 = 8,
	t1 = 9,
	t2 = 10,
	t3 = 11,
	t4 = 12,
	t5 = 13,
	t6 = 14,
	t7 = 15,
	t8 = 16, 
	t9 = 17,
} ARCH_REG;

ostream& operator<<( ostream &os, ARCH_REG & a);

typedef enum _registers {
        rINV = 0,          
        GP = 1,         
        FP = 2,          
        SP = 3          

} OFFSET_REGISTER;


class MemManUnit;
class Arg;

class MemoryLocation {
	unsigned int id;
	string name;
	ARCH_REG reg;
	bool in_mem;
	int size;
	OFFSET_REGISTER base_reg;
	long offset;
	bool is_ea;   

	friend MemoryLocation create_memory_location( string name, unsigned int id, long offset , int size );
	friend MemoryLocation create_temp_mem_location( string name, unsigned int id,  ARCH_REG reg , int size );
	friend void set_is_ea( unsigned int id );
	friend void process_arg( Arg * a);
	friend void issue_store( ARCH_REG r, MemoryLocation & ml );
	friend void issue_load( ARCH_REG r, MemoryLocation & ml );
	friend void issue_load_ea( ARCH_REG r, MemoryLocation & ml );
	friend void issue_load_ea( ARCH_REG r, ADDRESS & src );
	friend class MemManUnit;
public:
	MemoryLocation();
};
	
MemoryLocation create_memory_location( string name, unsigned int id, long offset , int size );
MemoryLocation create_temp_mem_location( string name, unsigned int id,  ARCH_REG reg , int size );
void set_is_ea( unsigned int id );

class MemManUnit {
public:
	int start_issue;
	vector<ARCH_REG> temp_stack;
	vector<unsigned int> reg_alloc_info;
	set<unsigned int> live_vals;
	map<unsigned int, MemoryLocation > memory_locations;
	map<string, string > strings;
	map<string, size_t> globals;
	size_t stack_size;
	long symtab_size;

	MemManUnit();
	ARCH_REG get_reg(ARCH_REG dest, Address* a, int mem_valid , bool load );
	ARCH_REG get_empty_reg( );
	void free_reg ( ARCH_REG reg );
	void store_and_free_reg ( ARCH_REG reg );
	void set_reg ( unsigned int table_id, ARCH_REG reg );
	void reset();
};

extern MemManUnit main_mem_unit;

typedef enum OP_CODE_ {
	INV = -1,
	LD,
	ST,
	ADD

} OP_CODE;

class AsmInstr {

	string name;
	ARCH_REG dest;
	ARCH_REG src1;
	ARCH_REG src2;
	uint16_t immm;
	OP_CODE  opc;
};
extern vector< AsmInstr > asm_code;

class Arg;
class Call;
class GoTo;
class Label;
class Return;
class SaveLive;
class Quad;

void gen_asm_code( );
void issue_load( ARCH_REG r, MemoryLocation & ml );
void issue_store( ARCH_REG r, MemoryLocation & ml );
void issue_load_ea( ARCH_REG r, MemoryLocation & ml );
void issue_load_ea( ARCH_REG r, ADDRESS & src );
void process_arg( Arg * a );
void process_call( Call * c );
void process_goto( GoTo * g );
void process_label( Label * l );
void process_quad( Quad * q );
void process_return( Return * r );
void process_save_live( SaveLive * s );
void gen_asm_instr_imm(string operation, ADDRESS & result, ADDRESS & arg1);
void gen_asm_instr_imm(string operation, ADDRESS & result, ADDRESS & arg1, ADDRESS & arg2);
void gen_asm_instr_limm(string operation, ADDRESS & result, ADDRESS & arg1, ADDRESS & arg2);
void gen_asm_instr(string operation, ADDRESS & result, ADDRESS & arg1);
void gen_asm_instr(string operation, ADDRESS & result, ADDRESS & arg1, ADDRESS & arg2);
void generate_footer();
void generate_header( );

#endif
