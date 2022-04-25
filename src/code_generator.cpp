#include<assert.h>
#include<code_generator.h>

std::vector<AsmInstr> asm_code;

MemManUnit main_mem_unit;

MemoryLocation::MemoryLocation() : id(0), name(""), reg(tINV), in_mem(true), size(0), base_reg(rINV), offset(0), is_ea(false) {}  

MemoryLocation create_memory_location(std::string name, unsigned int id,  long offset, int size ){
	MemoryLocation ml;
	ml.id = id;
	ml.size = size;
	ml.name = name;

	if ( id & TEMP_ID_MASK  ) {
		assert(0);
	} else if ( id & GLOBAL_SYM_MASK || id & STRING_MASK ) {
		ml.base_reg = GP;
		ml.offset = offset;
	} else if ( id & FUN_ARG_MASK ) {
		int arg_no = id & 0x0fffffff;
		switch (arg_no)
		{
		case 0: ml.reg = a0; ml.in_mem = false; break;
		case 1: ml.reg = a1; ml.in_mem = false; break;
		case 2: ml.reg = a2; ml.in_mem = false; break;
		case 3: ml.reg = a3; ml.in_mem = false; break;
		default:
			ml.base_reg = FP;
			ml.offset = 8 + 4*(arg_no - 4);
			break;
		}
	} else if ( id & LOCAL_SYM_MASK ){
		ml.base_reg = FP;
		ml.offset = offset;
	} else {
		assert(0);
	}
	return ml;
}

MemoryLocation create_temp_mem_location(std::string name, unsigned int id,  ARCH_REG reg , int size ){
	MemoryLocation ml;
	ml.id = id;
	ml.size = size;
	ml.name = name;

	if ( id & TEMP_ID_MASK  ) {
		ml.reg = reg;

	} else {
		assert(0);
	}
	return ml;
}

void set_is_ea( unsigned int id ){
	auto it = main_mem_unit.memory_locations.find( id );
	assert( it != main_mem_unit.memory_locations.end() );
	it->second.is_ea = true;
}


MemManUnit::MemManUnit() : start_issue(0), reg_alloc_info(std::vector<unsigned int>(NUM_ARCH_REGS, 0)) , stack_size(0), symtab_size(0) {} 

ARCH_REG MemManUnit::get_empty_reg() {
	int r = 0;
	while(r < NUM_TEMP_REGS){
		if ( reg_alloc_info[ (start_issue + r) % NUM_TEMP_REGS ] == 0 ) {
			ARCH_REG empty =  static_cast<ARCH_REG> ( t0 + ( (start_issue + r ) % NUM_TEMP_REGS ) );
			start_issue =  (start_issue + r + 1 ) % NUM_TEMP_REGS;
			return empty;
		}
		r++;
	}

	while(r < NUM_TEMP_REGS){
		if ( ( reg_alloc_info[ (start_issue + r) % NUM_TEMP_REGS ] & TEMP_ID_MASK )  == 0 ) {
			unsigned int table_id = reg_alloc_info[ (start_issue + r) % NUM_TEMP_REGS ]; 
			auto it = memory_locations.find( table_id );
			assert( it != memory_locations.end() );

			issue_store( it->second.reg, it->second );
			it->second.reg = tINV;
			it->second.in_mem = true;
			ARCH_REG empty =  static_cast<ARCH_REG> ( t0 +( (start_issue + r ) % NUM_TEMP_REGS) );
			start_issue =  (start_issue + r + 1 ) % NUM_TEMP_REGS;
			return empty;
		}
		r++;
	}

	r = 0;
	while(r < NUM_TEMP_REGS){
		if ( ( reg_alloc_info[ (start_issue + r) % NUM_TEMP_REGS ] & TEMP_ID_MASK )  != 0 ) {
			unsigned int table_id = reg_alloc_info[ (start_issue + r) % NUM_TEMP_REGS ]; 
			auto it = memory_locations.find( table_id );
			assert( it != memory_locations.end() );

			ARCH_REG empty =  static_cast<ARCH_REG> ( t0 +( (start_issue + r ) % NUM_TEMP_REGS) );
			stack_size += 4;
			asm_ss << "ASM: \t" << "addiu $sp, $sp, -4\n"; 
			it->second.base_reg = FP;
			it->second.offset = stack_size;
			it->second.size = WORD_SIZE;
			issue_store( empty, it->second );
			it->second.reg = tINV;
			it->second.in_mem = true;
			start_issue =  (start_issue + r + 1 ) % NUM_TEMP_REGS;
			return empty;
		}
		r++;
	}

	std::cerr << "PANIC: Unable to find empty register\n";
	assert(0);
	return tINV;

}

ARCH_REG MemManUnit::get_reg( ARCH_REG dest, Address * a , int mem_valid , bool load ) {
	auto it = memory_locations.find( a->table_id );
	if ( it != memory_locations.end() ) {
		if ( it->second.reg != tINV ) {
			if ( mem_valid == 1 ) {
				it->second.in_mem = true;
			} else if ( mem_valid == 0 ) {
				it->second.in_mem = false;
			}
			return it->second.reg;
		}

		ARCH_REG r;
		if ( dest == tINV || dest > t9 || dest < t0 ) {
			r = get_empty_reg();
		} else {
			r = dest;
		}
		assert( r != tINV && r <= t9  && r >= t0 );
		reg_alloc_info[r - t0 ] = a->table_id;
		it->second.reg = r;
		if ( load && it->second.in_mem && !it->second.is_ea ) {
			issue_load(r, it->second );
		} else if ( load && it->second.is_ea ) {
			issue_load_ea(r, it->second );
		} else if ( load ) {
			std::cerr << "PANIC : Loading value not in memory\n";
			assert(0);
		} 
		if ( mem_valid == 1 ) {
			it->second.in_mem = true;
		} else if ( mem_valid == 0 ) {
			it->second.in_mem = false;
		}
		return r;
		
	} 
	else if ( a->table_id & TEMP_ID_MASK ) {
		ARCH_REG r;
		if ( dest == tINV || dest > t9  || dest < t0) {
			r = get_empty_reg();
		} else {
			r = dest;
		}
		assert( r != tINV && r <= t9 && r >= t0 );
		reg_alloc_info[r - t0] = a->table_id;
		memory_locations.insert({a->table_id,create_temp_mem_location(a->name, a->table_id, r, a->size )});
		return r;
	} 
	else {
		std::cerr << "PANIC: Memory Location " << a->name << " not found\n";
		assert(0);
	}
	return tINV;
}

void MemManUnit::free_reg( ARCH_REG r ) {
	assert ( r != tINV );
	if ( r < t0 || r > t9 ){
		return;
	}
	unsigned int id = reg_alloc_info[r - t0];
	auto it = memory_locations.find( id ) ;
	assert ( it != memory_locations.end() );
	it->second.reg = tINV;
	reg_alloc_info[ r - t0 ] = 0;
}

void MemManUnit::store_and_free_reg( ARCH_REG r ) {
	assert ( r != tINV );
	if ( r < t0 || r > t9 ){
		return;
	}
	unsigned int id = reg_alloc_info[ r - t0 ];
	auto it = memory_locations.find( id ) ;
	assert ( it != memory_locations.end() );
	it->second.reg = tINV;
	if ( it->second.in_mem != true ) {
		issue_store( r, it->second );
	}
	it->second.in_mem = true;
	reg_alloc_info[r - t0] = 0;
}

void MemManUnit::reset() {
	start_issue = 0;
	temp_stack.clear();
	std::fill(reg_alloc_info.begin(), reg_alloc_info.end(), 0);
	live_vals.clear();
	memory_locations.clear();
	globals.clear();
	strings.clear();
	stack_size = 0;
	symtab_size = 0;
}

void issue_store( ARCH_REG r, MemoryLocation & ml ) {
	std::string store;
	if ( ml.size == 1 ) {
		store = "sb";
	}
	else if (ml.size == WORD_SIZE ) {
		store = "sw";
	} 
	else {
		assert(0);
	}
	
	if ( ml.base_reg == GP ) {
		asm_ss << "ASM: \t" << store << " " << r << ", " << ml.name << "\n";
	} 
	else if ( ml.base_reg == FP ) {
		asm_ss << "ASM: \t" << store << " " << r << ", " <<  -main_mem_unit.symtab_size + ml.offset << "($fp)\n"; 
	}
	else {
		 assert(0);
	}
}
void issue_load( ARCH_REG r, MemoryLocation & ml ) {
	std::string load;
	if ( ml.size == 1 ) {
		load = "lb";
	} else if (ml.size == WORD_SIZE ) {
		load = "lw";
	} else {
		assert(0);
	}

	if ( ml.base_reg == GP ) {
		asm_ss << "ASM: \t" << load << " " << r << ", " << ml.name << "\n";
	}
	else if ( ml.base_reg == FP ) {
		asm_ss << "ASM: \t" << load << " " << r << ", " <<  -main_mem_unit.symtab_size + ml.offset << "($fp)\n"; 
	} 
	else {
		assert(0);
	}
}

void issue_load_ea( ARCH_REG r, ADDRESS & src ) {
	auto it = main_mem_unit.memory_locations.find(src.addr->table_id);
	assert( it != main_mem_unit.memory_locations.end() );
	MemoryLocation & ml = it->second;
	if ( ml.base_reg == GP ) {
		asm_ss << "ASM: \t" << "la " << r << ", " << ml.name << "\n";
	}
	else if ( ml.base_reg == FP ) {
		asm_ss << "ASM: \t" << "addiu " << r << ", $fp " << -main_mem_unit.symtab_size + ml.offset << "\n"; 
	} 
}
void issue_load_ea( ARCH_REG r, MemoryLocation & ml ) {
	if ( ml.base_reg == GP ) {
		asm_ss << "ASM: \t" << "la " << r << ", " << ml.name << "\n";
	}
	else if ( ml.base_reg == FP ) {
		asm_ss << "ASM: \t" << "addiu " << r << ", $fp, " << -main_mem_unit.symtab_size + ml.offset << "\n"; 
	} 
}


void process_quad ( Quad * q ) {
	if ( q->arg1.addr != nullptr && q->arg2.addr != nullptr ) {
		if ( q->arg1.addr->type != CON && q->arg2.addr->type != CON ) {
			gen_asm_instr( q->operation, q->result, q->arg1, q->arg2 );
		}
		else if ( q->arg1.addr->type != CON && q->arg2.addr->type == CON ) {
			gen_asm_instr_imm( q->operation, q->result, q->arg1, q->arg2);
		}
		else if ( q->arg1.addr->type == CON && q->arg2.addr->type != CON ) {
			gen_asm_instr_limm( q->operation, q->result, q->arg1, q->arg2);

		} else {
			ARCH_REG dest = main_mem_unit.get_reg(tINV, q->result.addr, 0, false);
			if ( q->result.alive && q->result.next_use == nullptr ) {
				main_mem_unit.store_and_free_reg( dest );
			}
			;
		}


	} else if ( q->arg1.addr != nullptr ) {
		if ( q->arg1.addr->type != CON ) {
			gen_asm_instr( q->operation, q->result, q->arg1);

		} else {
			gen_asm_instr_imm( q->operation, q->result, q->arg1);
			;
		}
	}  
}


void gen_asm_instr(std::string operation, ADDRESS & result, ADDRESS & arg1, ADDRESS & arg2){
	if ( !result.alive ) {
		asm_ss << "ASM: xxxx\t";
	}
	ARCH_REG src1 = main_mem_unit.get_reg(tINV, arg1.addr , 2, true);
	ARCH_REG src2 = main_mem_unit.get_reg(tINV, arg2.addr , 2, true);
	
	ARCH_REG dest = tINV;
	if ( !arg2.alive ) {
		main_mem_unit.free_reg( src2 );
		dest = src2;
	} else if ( arg2.alive && arg2.next_use == nullptr ) {
		main_mem_unit.store_and_free_reg( src2 );
		dest = src2;
	}
	if ( !arg1.alive ) {
		main_mem_unit.free_reg( src1 );
		dest = src1;
	} 
	else if ( arg1.alive && arg1.next_use == nullptr ) {
		main_mem_unit.store_and_free_reg( src1 );
		dest = src1;
	}
	dest = main_mem_unit.get_reg( dest, result.addr, 0, false);

	if ( operation == "+" ) {
		asm_ss << "ASM: \t" << "addu" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	} else if ( operation == "-" ) {
		asm_ss << "ASM: \t" << "subu" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	} else if ( operation == "==" ) {
		asm_ss << "ASM: \t" << "seq" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	}  else if ( operation == "!=" ) {
		asm_ss << "ASM: \t" << "sne" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	}  else if ( operation == ">=" ) {
		asm_ss << "ASM: \t" << "sge" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	}  else if ( operation == "<=" ) {
		asm_ss << "ASM: \t" << "sle" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	}  else if ( operation == ">" ) {
		asm_ss << "ASM: \t" << "sgt" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	}  else if ( operation == "<" ) {
		asm_ss << "ASM: \t" << "slt" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	}  else if ( operation == "*" ) {
		asm_ss << "ASM: \t" << "mul" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	}  else if ( operation == "/" ) {
		asm_ss << "ASM: \t" << "div" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	}	else if ( operation == "|" ) {
		asm_ss << "ASM: \t" << "or" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	}	else if ( operation == "&" ) {
		asm_ss << "ASM: \t" << "and" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	}	else if ( operation == "^" ) {
		asm_ss << "ASM: \t" << "xor" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	} 	else if ( operation == ">>" ) {
		asm_ss << "ASM: \t" << "srav" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	}  else if ( operation == "<<" ) {
		asm_ss << "ASM: \t" << "sllv" << " " << dest <<", " << src1 << ", " << src2<<"\n";
	}else {
		std::cerr << "PANIC: unknown operation " << operation << "\n";
		assert(0);
	}

	if ( result.alive && result.next_use == nullptr ) {
		main_mem_unit.store_and_free_reg( dest );
	}

}
void gen_asm_instr_imm(std::string operation, ADDRESS & result, ADDRESS & arg1, ADDRESS & arg2){
	ARCH_REG src1 = main_mem_unit.get_reg(tINV, arg1.addr , 2, true);
	if ( !result.alive ) {
		asm_ss << "ASM: xxxx\t";
	}
	
	
	ARCH_REG dest = tINV;

	if ( !arg1.alive ) {
		main_mem_unit.free_reg( src1 );
		dest = src1;
	} 
	else if ( arg1.alive && arg1.next_use == nullptr ) {
		main_mem_unit.store_and_free_reg( src1 );
		dest = src1;
	}

	dest = main_mem_unit.get_reg( dest, result.addr, 0, false);

	int value = ( int ) std::stoi(arg2.addr->name);
	if ( value >= (int) -0x8000 && value < (int) 0x8000 ) {
		if ( operation == "+" ) {
			asm_ss << "ASM: \t" << "addiu" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		} else if ( operation == "-" ) {
			asm_ss << "ASM: \t" << "addiu" << " " << dest <<", " << src1 << ", " <<  (short) -value <<"\n";
		} else if ( operation == "==" ) {
			asm_ss << "ASM: \t" << "seq" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		}  else if ( operation == "!=" ) {
			asm_ss << "ASM: \t" << "sne" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		}  else if ( operation == ">=" ) {
			asm_ss << "ASM: \t" << "sge" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		}  else if ( operation == "<=" ) {
			asm_ss << "ASM: \t" << "sle" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		}  else if ( operation == ">" ) {
			asm_ss << "ASM: \t" << "sgt" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		}  else if ( operation == "<" ) {
			asm_ss << "ASM: \t" << "slt" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		}  else if ( operation == "*" ) {
			asm_ss << "ASM: \t" << "mul" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		}  else if ( operation == "/" ) {
			asm_ss << "ASM: \t" << "div" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		}  else if ( operation == ">>" ) {
			asm_ss << "ASM: \t" << "sra" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		}  else if ( operation == "<<" ) {
			asm_ss << "ASM: \t" << "sll" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		}  else if ( operation == "^" ) {
			asm_ss << "ASM: \t" << "xori" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		}  else if ( operation == "|" ) {
			asm_ss << "ASM: \t" << "ori" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		}  else if ( operation == "&" ) {
			asm_ss << "ASM: \t" << "andi" << " " << dest <<", " << src1 << ", " << (short) value <<"\n";
		} else {
			std::cerr << "PANIC: unknown operation " << operation << "\n";
			assert(0);
		}
	} else {
		asm_ss << "ASM: \t" << "li $v1, "  << value << "\n";
		if ( operation == "+" ) {
			asm_ss << "ASM: \t" << "addu" << " " << dest <<", " << src1 << ", " << static_cast<ARCH_REG>(v1) <<"\n";
		} else if ( operation == "-" ) {
			asm_ss << "ASM: \t" << "subu" << " " << dest <<", " << src1 << ", " << static_cast<ARCH_REG>(v1) <<"\n";
		} else if ( operation == "==" ) {
			asm_ss << "ASM: \t" << "seq" << " " << dest <<", " << src1 << ", " << static_cast<ARCH_REG>(v1) <<"\n";
		}  else if ( operation == "!=" ) {
			asm_ss << "ASM: \t" << "sne" << " " << dest <<", " << src1 << ", " << static_cast<ARCH_REG>(v1) <<"\n";
		}  else if ( operation == ">=" ) {
			asm_ss << "ASM: \t" << "sge" << " " << dest <<", " << src1 << ", " << static_cast<ARCH_REG>(v1) <<"\n";
		}  else if ( operation == "<=" ) {
			asm_ss << "ASM: \t" << "sle" << " " << dest <<", " << src1 << ", " << static_cast<ARCH_REG>(v1) <<"\n";
		}  else if ( operation == ">" ) {
			asm_ss << "ASM: \t" << "sgt" << " " << dest <<", " << src1 << ", " << static_cast<ARCH_REG>(v1) <<"\n";
		}  else if ( operation == "<" ) {
			asm_ss << "ASM: \t" << "slt" << " " << dest <<", " << src1 << ", " << static_cast<ARCH_REG>(v1) <<"\n";
		}  else if ( operation == "*" ) {
			asm_ss << "ASM: \t" << "mul" << " " << dest <<", " << src1 << ", " << static_cast<ARCH_REG>(v1) <<"\n";
		}  else if ( operation == "/" ) {
			asm_ss << "ASM: \t" << "div" << " " << dest <<", " << src1 << ", " << static_cast<ARCH_REG>(v1) <<"\n";
		}	else if ( operation == "^" ) {
			asm_ss << "ASM: \t" << "xor" << " " << dest <<", " << src1 << ", " << static_cast<ARCH_REG>(v1) <<"\n";
		}  else if ( operation == "|" ) {
			asm_ss << "ASM: \t" << "or" << " " << dest <<", " << src1 << ", " << static_cast<ARCH_REG>(v1) <<"\n";
		}  else if ( operation == "&" ) {
			asm_ss << "ASM: \t" << "and" << " " << dest <<", " << src1 << ", " << static_cast<ARCH_REG>(v1) <<"\n";
		} else {
			std::cerr << "PANIC: unknown operation " << operation << "\n";
			assert(0);
		}

	}

	if ( result.alive && result.next_use == nullptr ) {
		main_mem_unit.store_and_free_reg( dest );
	}




}

void gen_asm_instr_limm(std::string operation, ADDRESS & result, ADDRESS & arg1, ADDRESS & arg2){
	ARCH_REG src2 = main_mem_unit.get_reg(tINV, arg2.addr , 2, true);
	if ( !result.alive ) {
		asm_ss << "ASM: xxxx\t";
	}
	ARCH_REG dest = tINV;
	
	if ( arg2.alive && arg2.next_use == nullptr ) {
		main_mem_unit.store_and_free_reg( src2 );
		dest = src2;
	}
	else if ( !arg2.alive ) {
		main_mem_unit.free_reg( src2 );
		dest = src2;
	} 
	dest = main_mem_unit.get_reg( dest, result.addr, 0, false);

	int value = ( int ) std::stoi(arg1.addr->name);
	ARCH_REG src1 = tINV;
	if ( value == 0 ) {
		src1 = zz;
	} else {
		src1 = v1;
		asm_ss << "ASM: \t" << "li $v1, "  << value << "\n";
	}


	
	if ( operation == "+" ) {
		asm_ss << "ASM: \t" << "addu" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	} else if ( operation == "-" ) {
		asm_ss << "ASM: \t" << "subu" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	} else if ( operation == "==" ) {
		asm_ss << "ASM: \t" << "seq" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	}  else if ( operation == "!=" ) {
		asm_ss << "ASM: \t" << "sne" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	}  else if ( operation == ">=" ) {
		asm_ss << "ASM: \t" << "sge" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	}  else if ( operation == "<=" ) {
		asm_ss << "ASM: \t" << "sle" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	}  else if ( operation == ">" ) {
		asm_ss << "ASM: \t" << "sgt" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	}  else if ( operation == "<" ) {
		asm_ss << "ASM: \t" << "slt" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	}  else if ( operation == "*" ) {
		asm_ss << "ASM: \t" << "mul" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	}  else if ( operation == "/" ) {
		asm_ss << "ASM: \t" << "div" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	}	else if ( operation == "^" ) {
		asm_ss << "ASM: \t" << "xor" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	}  else if ( operation == "|" ) {
		asm_ss << "ASM: \t" << "or" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	}  else if ( operation == "&" ) {
		asm_ss << "ASM: \t" << "and" << " " << dest <<", " << src1 << ", " << src2 <<"\n";
	} else {
		std::cerr << "PANIC: unknown operation " << operation << "\n";
		assert(0);
	}

	if ( result.alive && result.next_use == nullptr ) {
		main_mem_unit.store_and_free_reg( dest );
	}


}

void gen_asm_instr(std::string operation, ADDRESS & result, ADDRESS & arg1){

	if ( operation != "()s" && !result.alive ) {
		asm_ss << "ASM: xxxx\t";
	}
	ARCH_REG src1 = main_mem_unit.get_reg(tINV, arg1.addr , 2, true);
	ARCH_REG dest = tINV;

	if ( !arg1.alive ) {
		main_mem_unit.free_reg( src1 );
		dest = src1;
	} else if ( arg1.alive && arg1.next_use == nullptr  ) {
		main_mem_unit.store_and_free_reg( src1 );
		dest = src1;
	}

	 if ( operation == "()s" ) {
		dest = main_mem_unit.get_reg( tINV, result.addr, 2, true);
		assert( dest != tINV );
	 } else {
		dest = main_mem_unit.get_reg( dest, result.addr, 0, false);
	}


	if ( operation == "=" ) {
		if ( dest != src1 ) {
			asm_ss << "ASM: \t" << "move" << " " << dest <<", " << src1 <<"\n";
		}
	} 
	else if ( operation == "la" ) {
		issue_load_ea(dest,arg1);
	}
	else if ( operation == "()" ) {
		if ( arg1.addr->size == 1 )
			asm_ss << "ASM: \t" << "lb" << " " << dest <<", 0(" << src1 << ")\n";
		else if ( arg1.addr->size == WORD_SIZE )
			asm_ss << "ASM: \t" << "lw" << " " << dest <<", 0(" << src1 << ")\n";
		else
			assert(0);
	}
	else if ( operation == "()s" ) {
		if ( result.addr->size == 1 ) {
			asm_ss << "ASM: \t" << "sb" << " " << src1 <<", 0(" << dest << ")\n";
		} else if ( result.addr->size == WORD_SIZE ) {
			asm_ss << "ASM: \t" << "sw" << " " << src1 <<", 0(" << dest << ")\n";
		} else {
			assert(0);
		}
	} else if ( operation == "!" ) {
		asm_ss << "ASM: \t" << "not" << " " << dest <<", " << src1 <<"\n";
	} else {
		std::cerr << "PANIC: unknown operation " << operation << "\n";
		assert(0);
	}

	if ( result.alive && result.next_use == nullptr ) {
		main_mem_unit.store_and_free_reg( dest );
	}
}

void gen_asm_instr_imm(std::string operation, ADDRESS & result , ADDRESS & arg1){
	if ( operation != "()s" && !result.alive ) {
		asm_ss << "ASM: xxxx\t";
	}
	ARCH_REG dest = tINV;
	if ( operation != "()s" ) {
		dest = main_mem_unit.get_reg( tINV, result.addr, 0, false);
	} 
	else {
		dest = main_mem_unit.get_reg( tINV, result.addr, 2, true);
		assert( dest != tINV );
	}
	int value = ( int ) std::stoi(arg1.addr->name);

	if ( operation == "=" ) {
			asm_ss << "ASM: \t" << "li "<< dest << ", " << value << "\n";
	} else if ( operation == "()s" ) {
		asm_ss << "ASM: \t" << "li $v1, " << value << "\n";
		if ( result.addr->size == 1 ) {
			asm_ss << "ASM: \t" << "sb" << " $v1, 0(" << dest << ")\n";
		} else if ( result.addr->size == WORD_SIZE ) {
			asm_ss << "ASM: \t" << "sw" << " $v1, 0(" << dest << ")\n";
		} else {
			assert(0);
		}

	} else {
		std::cerr << "PANIC: unknown operation " << operation << "\n";
		assert(0);
	}

	if ( result.alive && result.next_use == nullptr ) {
		main_mem_unit.store_and_free_reg( dest );
	}
	else if ( !result.alive  ) {
		main_mem_unit.free_reg( dest );
	}
}

void process_return( Return * r ) {
	if (r->retval.addr != nullptr) {
		if ( r->retval.addr->type != CON ) {
			ARCH_REG reg = main_mem_unit.get_reg( tINV, r->retval.addr, 2, true );
			asm_ss << "ASM: \t" << "move $v0, " << reg <<"\n";
			if ( r->retval.alive && r->retval.next_use == nullptr ) {
				main_mem_unit.store_and_free_reg( reg );
			}
			else if ( !r->retval.alive  ) {
				main_mem_unit.free_reg( reg );
			}
		} 
		else {
			int value = ( int ) std::stoi(r->retval.addr->name);
			asm_ss << "ASM: \t" << "li $v0, " << value << "\n";
		}
	}	
}


void process_arg( Arg * a) {
	if ( a->num < NUM_REG_ARGS ) {
		auto it = main_mem_unit.memory_locations.find( a->num | FUN_ARG_MASK );
		ARCH_REG arg_reg = static_cast<ARCH_REG>( a->num + a0 ); 
		if ( it != main_mem_unit.memory_locations.end() ) {
			main_mem_unit.temp_stack.push_back( arg_reg );
			asm_ss << "ASM: \t" << "addiu $sp, $sp, -4\n"; 
			asm_ss << "ASM: \t" << "sw " << arg_reg << ", " << 0 << "($sp)\n";
		}
		if ( a->arg.addr->type == CON ) {
			int value = std::stoi(a->arg.addr->name);
				asm_ss << "ASM: \t" << "li " << arg_reg << ", " << value << "\n";
		} else {
			ARCH_REG reg = main_mem_unit.get_reg( tINV, a->arg.addr, 2, true );
			if ( reg != arg_reg ) {
				asm_ss << "ASM: \t" << "move " << arg_reg << ", " << reg <<"\n";
			}
			if ( !a->arg.alive ) {
				main_mem_unit.free_reg( reg );
			} else if ( a->arg.alive && a->arg.next_use == nullptr ) {
				main_mem_unit.store_and_free_reg( reg );
			}
			return;
		}
	} else {
		if ( a->arg.addr->type == CON ) {
			int value = std::stoi(a->arg.addr->name);
			main_mem_unit.temp_stack.push_back( at );
			asm_ss << "ASM: \t" << "li $v1, " << value << "\n";
			asm_ss << "ASM: \t" << "addiu $sp, $sp, -4\n"; 
			if ( a->arg.addr->size == 1 ) {
				asm_ss << "ASM: \t" << "sb $v1, " << 0 << "($sp)\n";
			} else if ( a->arg.addr->size == WORD_SIZE ) {
				asm_ss << "ASM: \t" << "sw $v1, " << 0 << "($sp)\n";
			} else {
				assert(0);
			}
		} else {
			ARCH_REG reg = main_mem_unit.get_reg( tINV, a->arg.addr, 2, true );
			main_mem_unit.temp_stack.push_back( at );
			asm_ss << "ASM: \t" << "addiu $sp, $sp, -4\n"; 
			if ( a->arg.addr->size == 1 ) {
				asm_ss << "ASM: \t" << "sb " << reg << ", " << 0 << "($sp)\n";
			} 
			else if ( a->arg.addr->size == WORD_SIZE ) {
				asm_ss << "ASM: \t" << "sw " << reg << ", " << 0 << "($sp)\n";
			} 
			else
				assert(0);
			if ( !a->arg.alive ) {
				main_mem_unit.free_reg( reg );
			} else if ( a->arg.alive && a->arg.next_use == nullptr ) {
				main_mem_unit.store_and_free_reg( reg );
			}
			return;
		}
	} 
}

void process_goto( GoTo * g ) {
	if ( g->res.addr == nullptr ) {
		asm_ss << "ASM: \t" << "j _" << local_symbol_table.function_name << "_" << g->label->name << "_\n";
		return;
	} 
	assert(g->res.addr->type != CON );
	ARCH_REG reg = main_mem_unit.get_reg( tINV, g->res.addr, 2, true );
	asm_ss << "ASM: \t";
	if ( g->condition == true ) {
		asm_ss << "bnez ";
	} else {
		asm_ss << "beqz ";
	}
	asm_ss << reg << ", _"  << local_symbol_table.function_name << "_" << g->label->name << "_\n";
	if ( !g->res.alive ) {
		main_mem_unit.free_reg( reg );
	} else if ( g->res.alive && g->res.next_use == nullptr ) {
		main_mem_unit.store_and_free_reg( reg );
	}
}
void process_call( Call * c) {
	asm_ss << "ASM: \t" << "jal " << c->function_name << "\n";
	if ( c->retval.addr != nullptr && c->retval.alive == true ) {
			ARCH_REG reg = main_mem_unit.get_reg( tINV, c->retval.addr, 2, true );
			asm_ss << "ASM: \t" << "move" << " " << reg << ", $v0" << "\n";
	} 
	
	while ( !main_mem_unit.temp_stack.empty() ) {
		ARCH_REG r = main_mem_unit.temp_stack.back();
		if ( r != at ) {
			asm_ss << "ASM: \t" << "lw " << r << ", " << 0 << "($sp)\n";
		}
		asm_ss << "ASM: \t" << "addiu $sp, $sp, 4\n"; 
		main_mem_unit.temp_stack.pop_back();
	}
}


void process_save_live( SaveLive * s ){
	for ( int r = 0; r < NUM_TEMP_REGS; r++ ) {
		ARCH_REG reg = static_cast<ARCH_REG>( t0 + (r % NUM_TEMP_REGS) );
		if ( main_mem_unit.reg_alloc_info[ r % NUM_TEMP_REGS ] & TEMP_ID_MASK ) {
				if ( ! s->save_temps ) {
					continue;
				}
				assert ( reg != tINV && reg <= t9 && reg >= t0 );
				main_mem_unit.temp_stack.push_back( reg );
				asm_ss << "ASM: \t" << "addiu $sp, $sp, -4\n"; 
				asm_ss << "ASM: \t" << "sw " << reg << ", " << 0 << "($sp)\n";
		} else if ( main_mem_unit.reg_alloc_info[ r % NUM_TEMP_REGS ] != 0 ) {
			main_mem_unit.store_and_free_reg( reg );
		}
	}
}

void generate_header( ) {
	if ( !main_mem_unit.globals.empty() ) {
		asm_ss << "ASM: \t" << ".data\n";
		for ( auto i : main_mem_unit.globals ) {
			asm_ss << "ASM: "<< "\t.globl " << i.first << "\n";
			asm_ss << "ASM: "<< "\t.align 2\n";
			asm_ss << "ASM: " << i.first << ": " << "\t.space " << i.second << "\n"; 
		}
	}
	main_mem_unit.globals.clear();
	asm_ss << "ASM: \t" << "\n\n";
	asm_ss << "ASM: \t.text\n"; 
	asm_ss << "ASM: \t.globl " << local_symbol_table.function_name << "\n"; 
	asm_ss << "ASM: \t" << "\n\n";
	asm_ss << "ASM: " <<  local_symbol_table.function_name << ":\n";
	asm_ss << "ASM: \t" << "addiu $sp, $sp, " << -8 <<"\n";
	asm_ss << "ASM: \t" << "sw $ra, 4($sp)\n";
	asm_ss << "ASM: \t" << "sw $fp, 0($sp)\n";
	asm_ss << "ASM: \t" << "move $fp, $sp\n";
	main_mem_unit.symtab_size = local_symbol_table.reqd_size;
	main_mem_unit.stack_size = local_symbol_table.reqd_size < 32 ? 32 : local_symbol_table.reqd_size;
	size_t reqd_size = main_mem_unit.stack_size;
	if ( reqd_size <= 0x8000 ) {
		reqd_size = ( ~reqd_size + 1 ) & 0xffff;
		asm_ss << "ASM: \t" << "addiu $sp, $sp, " << (short) reqd_size << "\n";
	} else {
		asm_ss << "ASM: \t" << "li $v1, " << reqd_size << "\n";
		asm_ss << "ASM: \t" << "subu $sp, $sp, $v1\n";
	}
	asm_ss << "ASM: \t" << "\n";
}

void generate_footer() {
	asm_ss << "ASM: \t" << "move $sp, $fp\n";
	asm_ss << "ASM: \t" << "lw $fp, 0($sp)\n";
	asm_ss << "ASM: \t" << "lw $ra, 4($sp)\n";
	asm_ss << "ASM: \t" << "addiu $sp, $sp, " << 8 <<"\n";
	asm_ss << "ASM: \t" << "jr $ra\n";
	
	if ( !main_mem_unit.strings.empty() ) {
		asm_ss << "ASM: \t" << ".data\n";
		for ( auto i : main_mem_unit.strings ) {
			asm_ss << "ASM: " << i.first << ":\t.asciiz\t" << i.second << "\n"  ;
		}
	}
	main_mem_unit.strings.clear();

}

std::ostream& operator<<( std::ostream &os, ARCH_REG & a){
	switch ( a ) {
		case zz: os << "$0"; break;
		case t0: os << "$t0"; break;
		case t1: os << "$t1"; break;
		case t2: os << "$t2"; break;
		case t3: os << "$t3"; break;
		case t4: os << "$t4"; break;
		case t5: os << "$t5"; break;
		case t6: os << "$t6"; break;
		case t7: os << "$t7"; break;
		case t8: os << "$t8"; break;
		case t9: os << "$t9"; break;
		case a0: os << "$a0"; break;
		case a1: os << "$a1"; break;
		case a2: os << "$a2"; break;
		case a3: os << "$a3"; break;
		case v0: os << "$v0"; break;
		case v1: os << "$v1"; break;
		case tINV:
			os << "$tINV";
		default:
			assert(0);
	}
	return os;
}



void gen_asm_code( ){
	generate_header();
	for ( auto i:ta_code ) {
		if ( i->dead ) { 
			continue; 
		}
	
		Quad * q = dynamic_cast<Quad *>(i);
		if ( q != nullptr ) {
			process_quad(q);
			continue;
		}
		Label * l = dynamic_cast<Label *>(i);
		if ( l != nullptr ) {
			asm_ss << "ASM: " << "_" << local_symbol_table.function_name << "_" << l->name << "_:\n";
			continue;
		}
		Return * r = dynamic_cast<Return *>(i);
		if ( r != nullptr ) {
			process_return(r);
			continue;
		}
		Arg * a = dynamic_cast<Arg *>(i);
		if ( a != nullptr ) {
			process_arg(a);
			continue;
		}
		Call * c = dynamic_cast<Call *>(i);
		if ( c != nullptr ) {
			process_call(c);
			continue;
		}
		GoTo * g = dynamic_cast<GoTo *>(i);
		if ( g != nullptr ) {
			process_goto(g);
			continue;
		}
		SaveLive* s = dynamic_cast<SaveLive *>(i);
		if ( s != nullptr ) {
			process_save_live(s);
			continue;
		}
	}
	generate_footer();
	main_mem_unit.reset();

}
