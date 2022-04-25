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
#include <string.h>

#include "ast.h"
#include "expression.h"
#include "symtab.h"
#include "3ac.h"
#include "statement.h"
#include "y.tab.h"

using namespace std;

extern unsigned int line_no;
extern unsigned int col_no;

Type invalid_type;




Expression *create_primary_identifier( Identifier *a ) {
    PrimaryExpression *PE = new PrimaryExpression();
    PE->name = "primary_expression";
    PE->isTerminal = 1;
    PE->Ival = a;

    SymTabEntry *ste = local_symbol_table.get_symbol_from_table( a->value );
    if ( ste == nullptr ) {
        ste = global_symbol_table.get_symbol_from_table( a->value );
        if ( ste == nullptr ) {
            
            error_message( "Undeclared symbol " + a->value, a->line_no,
                       a->col_no );
            PE->type = invalid_type;
            return PE;
        }
    }
    PE->line_no = a->line_no;
    PE->col_no = a->col_no;
    PE->add_child( a );
    PE->type = ste->type;
    PE->res = new_3id( ste );

    return PE;
}

Expression *create_primary_constant(Constant *a) {
    PrimaryExpression *PE = new PrimaryExpression();
    PE->name = "primary_expression";
    PE->isTerminal = 2;
    PE->Cval = a;
    PE->line_no = a->line_no;
    PE->col_no = a->col_no;
    PE->add_child(a);
    int value = (int) a->val.i;
    PE->type = a->getConstantType();
    PE->res = new Address(value, CON);

    return PE;
}

Expression *create_primary_stringliteral(StringLiteral *a) {
    PrimaryExpression *PE = new PrimaryExpression();
    PE->name = "primary_expression";
    PE->isTerminal = 3;
    PE->Sval = a;
    PE->line_no = a->line_no;
    PE->col_no = a->col_no;
    PE->add_child( a );
	PE->type = Type(CHAR_T, 1, true);
    PE->res = new_3string(a); 
    return PE;
}


ArgumentExprList *create_argument_expr_assignement(Expression *ase) {
    ArgumentExprList *PE = new ArgumentExprList();
    PE->name = "arguments";
    PE->args.push_back(ase);
    PE->add_child(ase);
    return PE;
}
ArgumentExprList *create_argument_expr_list( ArgumentExprList *P, Expression *ase ) {
    P->name = "arguments";
    P->args.push_back( ase );
    P->add_child( ase );
    return P;
}

Expression *create_postfix_expr_arr( Expression *pe, Expression *e ) {
    PostfixExpression *PE = new PostfixExpression();
    if ( dynamic_cast<PostfixExpression *>( pe ) ) {
        PE->pe = dynamic_cast<PostfixExpression *>( pe );
    } else {
        PE->pe = nullptr;
    }
    PE->exp = e;
    PE->name = "ARRAY ACCESS";

    if (pe->type.is_invalid()) {
        PE->type = invalid_type;
        return PE;
    }
    if (e->type.is_invalid()) {
        PE->type = invalid_type;
        return PE;
    }

    if (!e->type.isInt()) {
        error_message("Array index must be of type integer", line_no);
        PE->type = invalid_type;
        return PE;
    }

	if (pe->type.is_arr) {
		PE->type = pe->type;
		PE->type.arr_size--;
		PE->type.ptr_level--;
		PE->type.is_const = false;
		PE->type.arr_sizes.erase( PE->type.arr_sizes.begin() );
	
		Address * t1;
		if (e->res->type != CON) {
			t1 = new_temp();
			emit( t1, "*", e->res, new_3const( PE->type.get_size() , INT3 ));
		} else {
			long off = stol(e->res->name)*PE->type.get_size();
			t1 = new_3const( off, INT3 );
		}

		if (PE->type.ptr_level == 0) {
			PE->type.is_pointer = false;
		}
		if ( PE->type.arr_size == 0 ) {
			PE->type.is_arr = 0;
		}
		
		PE->res = new_mem(PE->type);
		emit( PE->res, "+", pe->res, t1 );
		
	} else if ( pe->type.is_pointer ) {
		PE->type = pe->type;
		PE->type.is_const = false;
		PE->type.ptr_level--;
		if (!PE->type.ptr_level) PE->type.is_pointer = false;

		Address * t1;
		if (e->res->type != CON) {
			t1 = new_temp();
			emit( t1, "*", e->res, new_3const( PE->type.get_size() , INT3));
		} else {
			unsigned long long off = stol(e->res->name)*PE->type.get_size();
			t1 = new_3const(off, INT3);
		}
		
		PE->res = new_mem(PE->type);


		if (pe->res->type != MEM ) {
			emit( PE->res, "+", pe->res, t1 );
		} else {
			Address * t2 = new_mem(pe->type);
			emit(t2, "()", pe->res, nullptr);
			emit(PE->res, "+", t2, t1);
		}

	} else {
		error_message( "Subscripted value is neither array nor pointer", line_no );
		PE->type = invalid_type;
	}

    PE->line_no = pe->line_no;
    PE->col_no = pe->col_no;
    PE->add_children( pe, e );
    return PE;
}

Expression *create_postfix_expr_voidfun( Identifier *fi ) {
    PostfixExpression *PE = new PostfixExpression();

    SymTabEntry *ste = global_symbol_table.get_symbol_from_table( fi->value );
    if ( ste == nullptr ) {
        
        error_message( "Undeclared symbol " + fi->value, fi->line_no, fi->col_no );
        PE->type = invalid_type;
        return PE;
    } else if ( !ste->type.is_function ) {
        error_message( "Called object '" + fi->value + "' is not a function", fi->line_no, fi->col_no );
        PE->type = invalid_type;
        return PE;
    } else if ( ste->type.num_args != 0 ) {
        error_message( "Too few arguments to function '" + fi->value + "'", fi->line_no, fi->col_no );
        PE->type = invalid_type;
        return PE;
    }

    PE->name = "FUNCTION CALL";
    PE->line_no = fi->line_no;
    PE->col_no = fi->col_no;
    PE->add_child( fi );

    PE->type = ste->type;
    PE->type.is_function = false;
    PE->type.num_args = 0;
    PE->type.arg_types.clear();


    create_new_save_live();
	if ( !PE->type.isVoid() ) {
		PE->res = new_temp();
        create_new_call( PE->res, fi->value );
	} else {
        create_new_call( nullptr, fi->value );
	}

    return PE;
}

Expression *create_postfix_expr_fun( Identifier *fi, ArgumentExprList *ae ) {
    PostfixExpression *PE = new PostfixExpression();

    SymTabEntry *ste = global_symbol_table.get_symbol_from_table( fi->value );
    if ( ste == nullptr ) {
        error_message( "Undeclared symbol " + fi->value, fi->line_no, fi->col_no );
        PE->type = invalid_type;
        return PE;
    } else if ( !ste->type.is_function ) { 
        error_message( "Called object '" + fi->value + "' is not a function", fi->line_no, fi->col_no );
        PE->type = invalid_type;
        return PE;
    } else if ( ste->type.num_args > ae->args.size() ) {
        error_message( "Too few arguments to function '" + fi->value +
                       "'. Expected " + to_string( ste->type.num_args ) +
                       ", got " + to_string( ae->args.size() ),
                   fi->line_no, fi->col_no );
        PE->type = invalid_type;
        return PE;
    } else if ( ste->type.num_args < ae->args.size() ) {
        
        error_message( "Too many arguments to function '" + fi->value +
                       "'. Expected " + to_string( ste->type.num_args ) +
                       ", got " + to_string( ae->args.size() ),
                   fi->line_no, fi->col_no );
        PE->type = invalid_type;
        return PE;
    } else if ( ste->type.num_args == ae->args.size() ) {
        unsigned int i = 0;
        while ( i < ste->type.num_args) {
            if ( ae->args[i]->type.is_invalid() ) {
                PE->type = invalid_type;
                return PE;
            }

            if ( !( ste->type.arg_types[i] == ae->args[i]->type ) ) {
                error_message( "Type mismatch at argument " + to_string( i ) +
                               " of function '" + fi->value + "'. Expected " +
                               ste->type.arg_types[i].get_name() + ", got " +
                               ae->args[i]->type.get_name(),
                           fi->line_no, fi->col_no );
                PE->type = invalid_type;
                return PE;
            }
            i++;
        }
    }

    PE->name = "FUNCTION CALL";
    PE->add_children( fi, ae );
    PE->line_no = fi->line_no;
    PE->col_no = fi->col_no;
    PE->type = ste->type;
    PE->type.is_function = false;
    PE->type.num_args = 0;
    PE->type.arg_types.clear();



	unsigned int arg_count = ste->type.num_args < NUM_REG_ARGS ? ste->type.num_args : NUM_REG_ARGS; 
    unsigned int i = 0;
	while(i <  arg_count) {
	    Address * t1;
		MEM_EMIT(ae->args[i], t1);
        create_new_arg(t1, i );
        i++;
	}
    create_new_save_live();
    i = ste->type.num_args - 1;
	while (i >= NUM_REG_ARGS) {
	    Address * t1;
		MEM_EMIT(ae->args[i], t1);
        create_new_arg(t1, i );
        i--;
	}
	if ( !PE->type.isVoid() ) {
		PE->res = new_temp();
        create_new_call( PE->res, fi->value );
	} else {
        create_new_call( nullptr, fi->value );
	}

    return PE;
}

Expression *create_postfix_expr_struct( string access_op, Expression *pe, Identifier *i ) {
    PostfixExpression *PE = new PostfixExpression();

    if ( pe->type.is_invalid() ) {
        PE->type = invalid_type;
        return PE;
    }

    Types peT = defined_types[pe->type.typeIndex];
    if ( access_op == "." ) {
        if ( ( peT.is_struct || peT.is_union ) && pe->type.ptr_level == 0 ) {
            if ( peT.struct_definition == nullptr ) {
                error_message( i->value + " is not a member of " +
                               pe->type.get_name(),
                           i->line_no, i->col_no );
                PE->type = invalid_type;
                return PE;
            }
            Type *iType = peT.struct_definition->get_member( i );
            if ( iType == nullptr ) {
                error_message( i->value + " is not a member of " + peT.name, i->line_no, i->col_no );
                PE->type = invalid_type;
                return PE;
            } else {
                PE->type = *iType;
            }
        } else {
            error_message( "Invalid operand . with type " + pe->type.get_name(), i->line_no,i->col_no );
            PE->type = invalid_type;
            return PE;
        }
	size_t offset = peT.struct_definition->get_offset(i);
	if ( offset == 0 ) {
		PE->res = new_mem(PE->type);
		emit(PE->res,"=",pe->res,nullptr);
	}
	else {
		PE->res = new_mem(PE->type);
		emit( PE->res, "+", pe->res, new_3const( peT.struct_definition->get_offset(i) , INT3 )); 
	}

    } else if ( access_op == "->" ) {
        if ( (peT.is_union || peT.is_struct) & (pe->type.ptr_level == 1) ) {
            if (peT.struct_definition == nullptr) {
                error_message( i->value + " is not a member of " + peT.name, i->line_no, i->col_no );
                PE->type = invalid_type;
                return PE;
            }
            Type *iType = peT.struct_definition->get_member(i);
            if ( iType == nullptr ) {
                error_message( i->value + " is not a member of " + peT.name, i->line_no, i->col_no );
                PE->type = invalid_type;
                return PE;
            } else {
                PE->type = *iType;
            }
        } else {
            error_message( "Invalid operand -> with type " + pe->type.get_name(), i->line_no, i->col_no );
            PE->type = invalid_type;
            return PE;
        }
	size_t offset = peT.struct_definition->get_offset(i);
	if ( pe->res->type != MEM && offset == 0 ) {
		PE->res = new_mem(PE->type);
		emit(PE->res,"=",pe->res, nullptr);
	} else if ( pe->res->type != MEM && offset != 0 ){
		PE->res = new_mem(PE->type);
		emit( PE->res, "+", pe->res, new_3const( peT.struct_definition->get_offset(i) , INT3 )); 
	}
	else if ( offset == 0 ) {
		PE->res = new_mem(PE->type);
		emit(PE->res, "()", pe->res, nullptr);
	}
	else {
		Address * t1 = new_temp();
		emit(t1, "()", pe->res, nullptr);
		PE->res = new_mem(PE->type);
		emit( PE->res, "+", t1, new_3const( peT.struct_definition->get_offset(i) , INT3 )); 
	}
    }
    PE->name = access_op;
    PE->add_children( pe, i );
    return PE;
}

Expression *create_postfix_expr_ido( Terminal *op, Expression *pe ) {
    PostfixExpression *PE = new PostfixExpression();
    if ( dynamic_cast<PostfixExpression *>( pe ) ) {
        PE->pe = dynamic_cast<PostfixExpression *>( pe );
    } else {
        PE->pe = nullptr;
    }

    if ( pe->type.is_invalid() ) {
        PE->type = invalid_type;
        return PE;
    }

    PE->op = op->name;

    if ( op->name == "++" )
        PE->name = "POST INCREMENT";
    else {
        PE->name = "POST DECREMENT";
    }

    string op_code = op->name.substr( 0, 1 );

    Address *inc_value;
    
    if (  op->name != "++" && op->name != "--" ) {
		cerr << "PANIC: Invalid operation " << op->name <<"\n";
		assert(0);
		return PE;
    }
	if ( pe->type.is_const == true ) {
		error_message( "Invalid operand " + op->name + " with constant type", op->line_no, op->col_no );
		PE->type = invalid_type;
		return PE;
	}

	if ( pe->type.isPointer() ) {
		PE->type = pe->type;
		PE->res = new_mem(PE->type);
		Type t = pe->type;
		t.ptr_level--;
		inc_value = new_3const( t.get_size() , INT3);
	} else if ( pe->type.isInt() ) {
		PE->res = new_temp();
		PE->type = pe->type;
		inc_value = new_3const( 1, INT3 );
	} else if ( pe->type.isFloat() ) {
		PE->res = new_temp();
		PE->type = pe->type;
		inc_value = new_3const(1.0 , FLOAT3);
	} else {
		error_message( "Invalid operand " + op->name + " with type " + pe->type.get_name(),op->line_no, op->col_no );
		delete PE->res;
		PE->type = invalid_type;
		PE->res = nullptr;
		return PE;
	}

    PE->add_child( pe );

    if ( pe->res->type == MEM ) {
        Address *t1 = new_temp();
        emit( PE->res, "()", pe->res, nullptr );
        emit( t1, "=", PE->res, nullptr);
        emit( t1, op_code, t1, inc_value );
        emit( pe->res, "()s", t1, nullptr );
    } else if ( pe->res->type == ID3 ) {
        emit( PE->res, "=", pe->res, nullptr );
        emit( pe->res, op_code, pe->res, inc_value );
    } else {
		delete inc_value;
		delete PE->res;
		inc_value = nullptr;
		PE->res = nullptr;
        error_message( "lvalue required as operand to" + op->name, op->line_no, op->col_no + 1 );
        PE->type = invalid_type;
        return PE;
    }
    return PE;
}


Expression *create_unary_expression( Terminal *op, Expression *ue ) {
    UnaryExpression *UE = new UnaryExpression();
    UE->op1 = ue;
    UE->op = op->name;
    Type ueT = ue->type;
    if ( ueT.is_invalid() ) {
        UE->type = invalid_type;
        return UE;
    }
    string u_op = op->name;
    UE->name = u_op;
    Address *inc_value = nullptr;

    if ( u_op == "--" || u_op == "++" ) {
        if ( ueT.is_const == true ) {
            error_message( "Invalid operand " + u_op + " with constant type", op->line_no, op->col_no );
            UE->type = invalid_type;
            return UE;
        }
        
		u_op = u_op.substr( 0, 1 );
		if ( ue->type.isPointer() ) {
			UE->type = ue->type;
            UE->res = new_mem(UE->type);
			Type t = ue->type;
			t.ptr_level--;
			inc_value = new_3const( t.get_size() , INT3 );
		} else if ( ue->type.isInt() ) {
            UE->res = new_temp();
			UE->type = ue->type;
			inc_value = new_3const( 1 , INT3 );

		} else if ( ue->type.isFloat() ) {
            UE->res = new_temp();
			UE->type = ue->type;
			inc_value = new_3const( 1.0, FLOAT3 );
		} else {
			error_message( "Invalid operand " + u_op + " with type " + ue->type.get_name(),op->line_no, op->col_no );
			delete UE->res;
			UE->type = invalid_type;
			UE->res = nullptr;
			return UE;
		}


        if ( ue->res->type == MEM ) {
		Address * t1 = new_temp();
            emit( t1, "()", ue->res, nullptr );
            emit( UE->res, u_op, t1, inc_value );
            emit( ue->res, "()s", UE->res, nullptr );
        } else if ( ue->res->type == ID3 ) {
            UE->res = ue->res;
			ue->res->type = TEMP;
            emit( UE->res, u_op, UE->res, inc_value );
        } else {
			delete inc_value;
			inc_value = nullptr;
            error_message( "lvalue required as unary " + op->name + " operand", op->line_no, op->col_no );
            UE->type = invalid_type;
            return UE;
        }
    } else if ( u_op == "sizeof" ) {
        UE->name = "sizeof";
        UE->type = Type();
        UE->type.typeIndex = PrimitiveTypes::INT_T;
        UE->type.ptr_level = 0;
        UE->type.is_const = true;
        UE->res = new_3const( ue->type.get_size() , INT3);
    } else {
        cerr << "Error parsing Unary Expression.\n";
        cerr << "ERROR at line " << line_no << "\n";
        exit( 0 );
    }
    UE->add_child(ue);
    return UE;
}


// ##################################### Anjali ###########################################

Expression *create_unary_expression_cast( Node *n_op, Expression *ce ) {
    UnaryExpression *ue = new UnaryExpression();
    Terminal *t_op = dynamic_cast<Terminal *>( n_op );
    string u_op = t_op->name;
    ue->op = u_op;
    ue->op1 = ce;
    Type ceT = ce->type;
    if (ceT.is_invalid()) {
        ue->type = invalid_type;
        return ue;
    }
    if ( u_op == "!" ) {
        if ( ! ce->type.isInt() ) {
            error_message( "Invalid operand " + u_op + " on type " + ceT.get_name(),
                       n_op->line_no, n_op->col_no );
            ue->type = invalid_type;
            return ue;
        }
            ue->type = Type( U_CHAR_T, 0, 0 );
            Address *t1;
            MEM_EMIT( ce, t1 );
			ue->truelist = ce->falselist;
			ue->falselist = ce->truelist;
            BACKPATCH ( ue );
			ue->res = new_temp();
            emit( ue->res, u_op, t1, nullptr );
    } else if (u_op == "&") {
        if ( ceT.is_function ) {
            error_message( "lvalue required as unary & operand", n_op->line_no, n_op->col_no );
            ue->type = invalid_type;
            return ue;
        }
		ue->type.is_pointer = true;
		ue->type = ce->type;
		ue->type.ptr_level++;
		if ( ce->res->type == ID3 ) {
			ue->res = new_temp();
			emit ( ue->res, "la", ce->res, nullptr);
		} else if ( ce->res->type == MEM ){
            ue->res = new_temp();
			emit(ue->res,"=",ce->res,nullptr);
        } else {
		   error_message( "lvalue required as unary & operand", n_op->line_no,
				   n_op->col_no );
			ue->type = invalid_type;
			return ue; 
		}

    } else if (u_op == "*") {
        if(ce->type.is_arr) {
            error_message( "Cannot dereference type " + ceT.get_name(),
                       n_op->line_no, n_op->col_no );
            ue->type = invalid_type;
            return ue;
        }
        if ( ce->type.ptr_level < 0 ){
            error_message( "Cannot dereference type " + ceT.get_name(), n_op->line_no, n_op->col_no );
            ue->type = invalid_type;
            return ue;
        }
        if ( ce->type.ptr_level == 0 ) {
            error_message( "Cannot dereference type " + ceT.get_name(), n_op->line_no, n_op->col_no );
            ue->type = invalid_type;
            return ue;
        }        
		ue->type = ce->type;
		ue->type.ptr_level--;
		if ( ue->type.ptr_level == 1 ) {
			ue->type.is_pointer = true;
		} else {
			ue->type.is_pointer = false;
		}
		create_new_save_live(false);
		if ( ce->res->type == ID3 ) {
			ue->res = new_mem(ue->type);
			emit(ue->res, "=", ce->res, nullptr );
		} else if ( ce->res->type == MEM ){
            ue->res = new_mem(ue->type);
			emit( ue->res, "()", ce->res, nullptr );
        } else {
			error_message( "lvalue required as unary * operand", n_op->line_no,
					   n_op->col_no );
			ue->type = invalid_type;
			return ue;
		}

    } else if ( u_op == "+" ) {
        if ( ce->type.isIntorFloat() == false ) {
            error_message( "Invalid operand " + u_op + " on type " + ceT.get_name(),
                       n_op->line_no, n_op->col_no );
            ue->type = invalid_type;
            return ue;
        }

		ue->type = ce->type;
		ue->type.make_signed();
		Address *t1;
		MEM_EMIT( ce, t1 );
		ue->res = new_temp();
		Address * zero_value = nullptr;
		if ( ce->type.isInt() == true ) {
			zero_value = new_3const( 0, INT3 );
		} 
		else if ( ce->type.isFloat() == true ) {
			zero_value = new_3const( 0.0 , FLOAT3 );
		} else {
			assert(0);
		}
		emit( ue->res, u_op, zero_value, t1 );
    }  else if ( u_op == "-" ) {
        if ( ce->type.isIntorFloat() == false ) {
            error_message( "Invalid operand " + u_op + " on type " + ceT.get_name(),
                       n_op->line_no, n_op->col_no );
            ue->type = invalid_type;
            return ue;
        }

		ue->type = ce->type;
		ue->type.make_signed();
		Address *t1;
		MEM_EMIT( ce, t1 );
		ue->res = new_temp();
		Address * zero_value = nullptr;
		if ( ce->type.isInt() == true ) {
			zero_value = new_3const( 0, INT3 );
		} 
		else if ( ce->type.isFloat() == true ) {
			zero_value = new_3const( 0.0 , FLOAT3 );
		} else {
			assert(0);
		}
		emit( ue->res, u_op, zero_value, t1 );
    } else {
        cerr << "Parse error, invalid unary operator\n";
        cerr << "ERROR at line " << line_no << "\n";
        exit( 0 );
    }

    ue->name = "unary_expression";
    ue->add_children( n_op, ce );
    return ue;
}

Expression *create_unary_expression( Terminal *op, TypeName *t_name ) {
    UnaryExpression *UE = new UnaryExpression();
    string u_op = op->name;
    UE->add_children( op, t_name );
    UE->type = Type( PrimitiveTypes::INT_T, 0, true );
    UE->res = new_3const( t_name->type.get_size() , INT3);
    UE->name = u_op;
    return UE;
}

Expression *create_cast_expression_typename( TypeName *tn, Expression *ce ) {
    CastExpression *CCET = new CastExpression();
    CCET->op1 = ce;
    Type ceT = ce->type;
    Type tnT = tn->type;
    if ( ceT.is_invalid() ) {
        CCET->type = invalid_type;
        return CCET;
    }
    if ( tnT.is_invalid() ) {
        CCET->type = invalid_type;
        return CCET;
    }
    if ( ce->type.isIntorFloat() && tn->type.isIntorFloat()){
        
            CCET->type = tn->type;
        
    } else if ( ce->type.ptr_level > 0 && tn->type.ptr_level > 0) {
       
            CCET->type = tn->type;
        
    } else {
        error_message( "Undefined casting operation of " + ceT.get_name() + " into " + tnT.get_name(),
                   line_no );
        CCET->type = invalid_type;
        return CCET;
    }
	Address * t1;
	MEM_EMIT( ce, t1 );
    CCET->name = "cast_expression";
	CCET->res = t1;
    CCET->add_children( tn, ce );
    return CCET;
}
Expression *create_multiplicative_expression( string op, Expression *me, Expression *ce ) {
    MultiplicativeExpression *P = new MultiplicativeExpression();
    P->op = op;
    P->op1 = me;
    P->op2 = ce;
    Type ceT = ce->type;
    Type meT = me->type;
    if ( meT.is_invalid() ) {
        P->type = invalid_type;
        return P;
    }
    if( ceT.is_invalid() ) {
        P->type = invalid_type;
        return P;
    }
    if( op == "/" ){
        if ( meT.isInt() && ceT.isInt() ) {

            if ( !meT.isUnsigned() && !ceT.isUnsigned() ) {
                ;
            } else if ( meT.isUnsigned() && ceT.isUnsigned() ) {
                op += "u";
            } else if ( !meT.isUnsigned() && ceT.isUnsigned() ) {
                meT.make_unsigned();
                op += "u";
            } else if ( meT.isUnsigned() && !ceT.isUnsigned() ) {
                ceT.make_unsigned();
                op += "u";
            }

            Address *t1, *t2;
            MEM_EMIT( me, t1 );
            MEM_EMIT( ce, t2 );
            P->res = new_temp();
            if( meT.typeIndex == ceT.typeIndex ) {
                P->type = meT;
            }
            if ( meT.typeIndex < ceT.typeIndex ) {
                P->type = ceT;
            } 
            if ( meT.typeIndex > ceT.typeIndex ) {
                P->type = meT;
            }
                emit( P->res, op, t1, t2 );
            
        } 
        else if ( ( meT.isFloat() && ceT.isInt() ) || ( meT.isInt() && ceT.isFloat() ) ) {
            P->type = meT.isFloat() ? meT : ceT;
        }
        else if ( meT.isFloat() && ceT.isFloat() ){
            
            op += "f";
            Address *t1, *t2;
            MEM_EMIT( me, t1 );
            MEM_EMIT( ce, t2 );
            SAVE_REGS ( P, t1, t2 );
            if ( meT.typeIndex == ceT.typeIndex ) {
                    P->type = meT;
            }
            else if ( meT.typeIndex < ceT.typeIndex ) {
                P->type = ceT;
            } else if ( meT.typeIndex > ceT.typeIndex ) {
                P->type = meT;
            }
            emit( P->res, op, t1, t2 );
            
        } else {
            error_message( "Undefined operation " + op + " for operands of type " +
                           meT.get_name() + " and " + ceT.get_name(),
                       line_no );
            P->type = invalid_type;
            return P;
        }
    } else if ( op == "*" ) {
        if ( meT.isInt() && ceT.isInt() ) {

            if ( !meT.isUnsigned() && !ceT.isUnsigned() ) {
                ;
            } else if ( meT.isUnsigned() && ceT.isUnsigned() ) {
                op += "u";
            } else if ( !meT.isUnsigned() && ceT.isUnsigned() ) {
                meT.make_unsigned();
                op += "u";
            } else if ( meT.isUnsigned() && !ceT.isUnsigned() ) {
                ceT.make_unsigned();
                op += "u";
            }

            Address *t1, *t2;
            MEM_EMIT( me, t1 );
            MEM_EMIT( ce, t2 );
            P->res = new_temp();
            if( meT.typeIndex == ceT.typeIndex ) {
                P->type = meT;
            }
            if ( meT.typeIndex < ceT.typeIndex ) {
                P->type = ceT;
            } 
            if ( meT.typeIndex > ceT.typeIndex ) {
                P->type = meT;
            }
                emit( P->res, op, t1, t2 );
            
        } 
        else if ( ( meT.isFloat() && ceT.isInt() ) || ( meT.isInt() && ceT.isFloat() ) ) {
            P->type = meT.isFloat() ? meT : ceT;
        }
        else if ( meT.isFloat() && ceT.isFloat() ){
            
            op += "f";
            Address *t1, *t2;
            MEM_EMIT( me, t1 );
            MEM_EMIT( ce, t2 );
            SAVE_REGS ( P, t1, t2 );
            if ( meT.typeIndex == ceT.typeIndex ) {
                    P->type = meT;
            }
            else if ( meT.typeIndex < ceT.typeIndex ) {
                P->type = ceT;
            } else if ( meT.typeIndex > ceT.typeIndex ) {
                P->type = meT;
            }
            emit( P->res, op, t1, t2 );
            
        } else {
            error_message( "Undefined operation " + op + " for operands of type " +
                           meT.get_name() + " and " + ceT.get_name(),
                       line_no );
            P->type = invalid_type;
            return P;
        }
    } else if ( op == "%" ) {

        if (  meT.isInt() == false ) {
            
            error_message( "Invalid operands to " + op + " for operands of type " +
                           meT.get_name() + " and " + ceT.get_name(),
                       line_no );
            P->type = invalid_type;
            return P;
        }
        
        if ( ceT.isInt() == false ) {
            error_message( "Invalid operands to " + op + " for operands of type " +
                           meT.get_name() + " and " + ceT.get_name(),
                       line_no );
            P->type = invalid_type;
            return P;
        }

		P->type = ceT;
		P->type.make_unsigned();
		P->res = new_temp();
		Address *t1, *t2;
		MEM_EMIT( me, t1 );
		MEM_EMIT( ce, t2 );
		emit( P->res, op, t1, t2 );

    } else {
		assert(0);
	}
    P->name = "multiplicative expression";
    Node *n_op = create_non_term( ( op ).c_str() );
    P->add_children( me, n_op, ce );

    return P;
}
Expression *create_additive_expression( string op, Expression *ade,
                                        Expression *me ) {
    AdditiveExpression *P = new AdditiveExpression();
    P->op = op;
    P->op1 = ade;
    P->op2 = me;
    Type meT = me->type;
    Type adeT = ade->type;
    if ( meT.is_invalid() ) {
        P->type = invalid_type;
        return P;
    }
    if ( adeT.is_invalid() ) {
        P->type = invalid_type;
        return P;
    }
    if ( meT.isInt() && adeT.isInt() ) {
        
        if ( !adeT.isUnsigned() && !meT.isUnsigned() ) {
            ;
        } else if ( adeT.isUnsigned() && !meT.isUnsigned() ) {
            meT.make_unsigned();
        } else if ( !adeT.isUnsigned() && meT.isUnsigned() ) {
            adeT.make_unsigned();
        } else if ( adeT.isUnsigned() && meT.isUnsigned() ) {
            ;
        } 

        Address *t1, *t2;
        MEM_EMIT( ade, t1 );
        MEM_EMIT( me, t2 );
        P->res = new_temp();
        if ( adeT.typeIndex == meT.typeIndex ) {
            P->type = adeT;
        }
        if ( adeT.typeIndex < meT.typeIndex ) {
            P->type = meT;
        } else if ( adeT.typeIndex > meT.typeIndex ) {
            P->type = adeT;
        }
        emit( P->res, op, t1, t2 );

        
    }  else if ( meT.isFloat() && adeT.isFloat() ) {
        error_message("Can't perform addition operation on operands of type " + meT.get_name() + " and " + adeT.get_name(), line_no);
        P->type = invalid_type;
        op += "f";
    } else if ( ( meT.isFloat() && adeT.isInt() ) ||  ( meT.isInt() && adeT.isFloat() ) ) {
        error_message("Can't perform addition operation on operands of type " + meT.get_name() + " and " + adeT.get_name(), line_no);
        P->type = invalid_type;
    }  else if ( meT.isPointer() && adeT.isInt() ) {
        
        P->type = meT;
        Address *t1;
        MEM_EMIT( ade, t1 );
        Type t = meT;
        t.ptr_level--;
        P->res = new_mem(P->type);
        Address * t2 = new_temp();
        emit( t2, "*", t1, new_3const( t.get_size() , INT3 ));
        emit( P->res, op, me->res, t2 );
        P->name = "additive_expression";
        Node *n_op = create_non_term( ( op ).c_str() );
        P->add_children( ade, n_op, me );
        return P;
        
    } else if ( adeT.isPointer() && meT.isInt() ){
        
        P->type = adeT;
        Address *t1;
        MEM_EMIT( me, t1 );
        P->res = new_mem(P->type);
        Type t = adeT;
        t.ptr_level--;
        Address * t2 = new_temp();
        emit( t2, "*", t1, new_3const( t.get_size() , INT3 ));
        emit( P->res, op, ade->res, t2 );
        P->name = "additive_expression";
        Node *n_op = create_non_term( ( op ).c_str() );
        P->add_children( ade, n_op, me );
        return P;
        
    } 
    else {        
        error_message( "Undefined operation " + op + " for operands of type " + meT.get_name() + " and " + adeT.get_name(), line_no );
        P->type = invalid_type;
        return P;
    }
    P->name = "additive_expression";
    Node *n_op = create_non_term( ( op ).c_str() );
    P->add_children( ade, n_op, me );
    return P;
}
Expression *create_shift_expression( string op, Expression *se,
                                     Expression *ade ) {
    ShiftExpression *CSE = new ShiftExpression();
    CSE->op = op;
    CSE->op1 = se;
    CSE->op2 = ade;

    Type adeT = ade->type;
    Type seT = se->type;

    if ( seT.is_invalid() ) {
        CSE->type = invalid_type;
        return CSE;
    }
    if ( adeT.is_invalid() ) {
        CSE->type = invalid_type;
        return CSE;
    }
    if ( op == "<<" || op == ">>" ) {
        if ( adeT.isInt() && seT.isInt() ) {
            CSE->type = seT;
            if ( seT.isUnsigned() ) {
                op += "u";
            }
        } else {
            error_message( "Undefined operation " + op + " for operands of type " +
                           seT.get_name() + " and " + adeT.get_name(),
                       line_no );
            CSE->type = invalid_type;
            return CSE;
        }
    } else {
        cerr << "Incorrect shift expression. Something went wrong\n";
        exit( 0 );
    }

    Node *n_op = create_non_term( ( op ).c_str() );
    CSE->name = "shift_expression";
    CSE->add_children( se, n_op, ade );
	CSE->res = new_temp();
    Address *t1, *t2;
    MEM_EMIT( ade, t2 );
    MEM_EMIT( se, t1 );
    emit( CSE->res, op, t1, t2 );
    return CSE;
}

Expression *create_relational_expression( string op, Expression *re,
                                          Expression *se ) {
    RelationalExpression *CRE = new RelationalExpression();
    CRE->op = op;
    CRE->op1 = re;
    CRE->op2 = se;
    Type reT = re->type;
    Type seT = se->type;
    if ( seT.is_invalid() ) {
        CRE->type = invalid_type;
        return CRE;
    }
    if ( reT.is_invalid() ) {
        CRE->type = invalid_type;
        return CRE;
    }    
    if ( op == "<=" || op == ">=" ) {
        if ( ( reT.isInt() || reT.isFloat() ) &&
             ( seT.isInt() || reT.isFloat() ) ) {
            CRE->type = Type( U_CHAR_T, 0, 0 );
            if ( reT.isUnsigned() != seT.isUnsigned() ) {
                warning_message( "Comparison " + op + " for operands of type " +
                                 reT.get_name() + " and " + seT.get_name(),
                             line_no );
            }
        } else {            
            error_message( "Undefined operation " + op + " for operands of type " +
                           reT.get_name() + " and " + seT.get_name(),
                       line_no );
            CRE->type = invalid_type;
            return CRE;
        }
    } 
    else if( op == ">" || op == "<" ){
        if ( ( reT.isInt() || reT.isFloat() ) &&
             ( seT.isInt() || reT.isFloat() ) ) {
            CRE->type = Type( U_CHAR_T, 0, 0 );
            if ( reT.isUnsigned() != seT.isUnsigned() ) {
                warning_message( "Comparison " + op + " for operands of type " +
                                 reT.get_name() + " and " + seT.get_name(),
                             line_no );
            }
        } else {            
            error_message( "Undefined operation " + op + " for operands of type " +
                           reT.get_name() + " and " + seT.get_name(),
                       line_no );
            CRE->type = invalid_type;
            return CRE;
        }
    }
    else {
        cerr << "Incorrect relation expression. Something went wrong\n";
        exit( 0 );
    }
    Node *n_op = create_non_term( ( op ).c_str() );
    CRE->name = "relational_expression";
    CRE->add_children( re, n_op, se );
	CRE->res = new_temp();
    Address *t1, *t2;
    MEM_EMIT( se, t2 );
    MEM_EMIT( re, t1 );
    emit( CRE->res, op, t1, t2 );
    return CRE;
}
Expression *create_equality_expression( string op, Expression *eq,
                                        Expression *re ) {
    EqualityExpression *CEE = new EqualityExpression();
    CEE->op = op;
    CEE->op1 = eq;
    CEE->op2 = re;
    Type reT = re->type;
    Type eqT = eq->type;

    if ( reT.is_invalid() ) {
        CEE->type = invalid_type;
        return CEE;
    }
    if ( eqT.is_invalid() ) {
        CEE->type = invalid_type;
        return CEE;
    }

    if ( op == "=="  ) {
        if ( reT.ptr_level > 0 && eqT.ptr_level > 0 ) {
            CEE->type = Type( U_CHAR_T, 0, 0 );
        } else if ( ( reT.ptr_level == 0 && eqT.ptr_level == 0 ) && ( reT.isInt() || reT.isFloat() ) && ( eqT.isInt() || eqT.isFloat()) ) {

            CEE->type = Type( U_CHAR_T, 0, 0 );

            if ( eqT.isUnsigned() != reT.isUnsigned() ) {
                warning_message( "Comparison " + op + " for operands of type " +
                                 eqT.get_name() + " and " + reT.get_name(),
                             line_no );
            }
        }

        else {
            
            error_message( "Undefined operation " + op + " for operands of type " +
                           eqT.get_name() + " and " + reT.get_name(),
                       line_no );
            CEE->type = invalid_type;
            return CEE;
        }
    } else if ( op == "!="){
        if ( reT.ptr_level > 0 && eqT.ptr_level > 0 ) {
            CEE->type = Type( U_CHAR_T, 0, 0 );
        } else if ( ( reT.ptr_level == 0 && eqT.ptr_level == 0 ) && ( reT.isInt() || reT.isFloat() ) && ( eqT.isInt() || eqT.isFloat()) ) {

            CEE->type = Type( U_CHAR_T, 0, 0 );

            if ( eqT.isUnsigned() != reT.isUnsigned() ) {
                warning_message( "Comparison " + op + " for operands of type " +
                                 eqT.get_name() + " and " + reT.get_name(),
                             line_no );
            }
        }

        else {
            
            error_message( "Undefined operation " + op + " for operands of type " +
                           eqT.get_name() + " and " + reT.get_name(),
                       line_no );
            CEE->type = invalid_type;
            return CEE;
        }
    } else {
        cerr << "Incorrect equality expression. Something went wrong\n";
        exit( 0 );
    }

    Node *n_op = create_non_term( ( op ).c_str() );
    CEE->name = "eqality_expression";
	CEE->res = new_temp();
    CEE->add_children( eq, n_op, re );
    Address *t1, *t2;
    MEM_EMIT( eq, t1 );
    MEM_EMIT( re, t2 );
    emit( CEE->res, op, t1, t2 );
    return CEE;
}

Expression *create_and_expression( string op, Expression *an,
                                   Expression *eq ) {
    AndExpression *CAE = new AndExpression();
    CAE->op = op;
    CAE->op1 = an;
    CAE->op2 = eq;
    Type anT = an->type;
    Type eqT = eq->type;

    if ( anT.is_invalid() ) {
        CAE->type = invalid_type;
        return CAE;
    }
    if( eqT.is_invalid() ){
        CAE->type = invalid_type;
        return CAE;
    }
    if ( op == "&" ) {
        if ( anT.isInt() && eqT.isInt() ) {
            if(anT.typeIndex > eqT.typeIndex) {
                CAE->type = anT;
            }
            else {
                CAE->type = eqT;
            }
            if ( !anT.isUnsigned() ) {
                CAE->type.make_signed();
            } 
            if ( !eqT.isUnsigned() ) {
                CAE->type.make_signed();
            }
        } else {            
            error_message( "Undefined operation " + op + " for operands of type " +
                           anT.get_name() + " and " + eqT.get_name(),
                       line_no );
            CAE->type = invalid_type;
            return CAE;
        }
    } else {
        cerr << "Incorrect and_expression. Something went wrong\n";
        exit( 0 );
    }

    Node *n_op = create_non_term( ( op ).c_str() );
    CAE->name = "and_expression";
    CAE->add_children( an, n_op, eq );
	CAE->res = new_temp();
    Address *t1, *t2;
    MEM_EMIT( an, t1 );
    MEM_EMIT( eq, t2 );
    emit( CAE->res, op, t1, t2 );
    return CAE;
}

// ############################### Lipi #############################################

Expression *create_exclusive_or_expression( string op, Expression *ex, Expression *an ) {
    ExclusiveorExpression *EOE = new ExclusiveorExpression();
    EOE->name = "exclusive_or_expression";
    EOE->op = op;
    EOE->op1 = ex;
    EOE->op2 = an;
    
    Type anT = an->type;
    if ( anT.is_invalid() ) {
        EOE->type = invalid_type;
        return EOE;
    }

    Type exT = ex->type;
    if ( exT.is_invalid() ) {
        EOE->type = invalid_type;
        return EOE;
    }

    if ( op == "^" ) {
        if ( exT.isInt() && anT.isInt() ) {
            if(anT.typeIndex > exT.typeIndex) {
                EOE->type = anT;
            }
            else {
                EOE->type = exT;
            }
            if (  !anT.isUnsigned() ) {
                EOE->type.make_signed();
            }
            if (  !exT.isUnsigned() ) {
                EOE->type.make_signed();
            }
        } else {
            EOE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + exT.get_name() + " and " + anT.get_name(), line_no );
            return EOE;
        }
    } else {
        cerr << "Incorrect exclusive or expression. Something went wrong\n";
        exit( 0 );
    }

    Node *node_op = create_non_term( ( op ).c_str() );
    EOE->add_children( ex, node_op, an );
    Address *t1, *t2;
    MEM_EMIT( ex, t1 );
    MEM_EMIT( an, t2 );
	EOE->res = new_temp();
    emit( EOE->res, op, t1, t2 );
    return EOE;
}

Expression *create_inclusive_or_expression( string op, Expression *ie, Expression *ex ) {
    InclusiveorExpression *IOE = new InclusiveorExpression();
    IOE->name = "inclusive_or_expression";
    IOE->op = op;
    IOE->op1 = ie;
    IOE->op2 = ex;

    Type ieT = ie->type;    
    if ( ieT.is_invalid() ) {
        IOE->type = invalid_type;
        return IOE;
    }

    Type exT = ex->type;
    if ( exT.is_invalid() ) {
        IOE->type = invalid_type;
        return IOE;
    }

    if ( op == "|" ) {
        if ( exT.isInt() && ieT.isInt() ) {
            if ( ieT.typeIndex > exT.typeIndex ) {
                IOE->type = ieT;
            }
            else {
                IOE->type =  exT;
            }
            if ( !ieT.isUnsigned() ) {
                IOE->type.make_signed();
            }
            if ( !exT.isUnsigned() ) {
                IOE->type.make_signed();
            }
            
        } else {
            IOE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + ieT.get_name() + " and " + exT.get_name(), line_no );
            return IOE;
        }
    } else {
        cerr << "Incorrect inclusive or expression. Something went wrong\n";
        exit( 0 );
    }

    Node *node_op = create_non_term( ( op ).c_str() );
    IOE->add_children( ie, node_op, ex );
    Address *t1, *t2;
    MEM_EMIT( ie, t1 );
    MEM_EMIT( ex, t2 );
	IOE->res = new_temp();
    emit( IOE->res, op, t1, t2 );
    return IOE;
}

Expression *create_logical_and_expression( string op, Expression *la, Expression *ie ) {
    Logical_andExpression *LAE = new Logical_andExpression();
    LAE->name = "logical_and_expression";
    LAE->op = op;
    LAE->op1 = la;
    LAE->op2 = ie;

    Type laT = la->type;
    if ( laT.is_invalid() ) {
        LAE->type = invalid_type;
        return LAE;
    }

    Type ieT = ie->type;
    if ( ieT.is_invalid() ) {
        LAE->type = invalid_type;
        return LAE;
    }

    if ( op == "&&" ) {
        if ( laT.isInt() &&  ieT.isInt() ) {
            LAE->type = Type( U_CHAR_T, 0, 0 );    
        } else {
            LAE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + laT.get_name() + " and " + ieT.get_name(), line_no );
            return LAE;
        }
    } else {
        cerr << "Incorrect logical and expression. Something went wrong\n";
        exit( 0 );
    }
    
    Node *node_op = create_non_term( ( op ).c_str() );
    LAE->add_children( la, node_op, ie );
    LAE->res = ie->res;    
	append(LAE->truelist,ie->truelist);
    append(LAE->falselist,la->falselist);
    append(LAE->falselist,ie->falselist);
    return LAE;
}

Expression *create_logical_or_expression( string op, Expression *lo, Expression *la ) {
    Logical_orExpression *LOE = new Logical_orExpression();
    LOE->name = "logical_or_expression";
    LOE->op = op;
    LOE->op1 = lo;
    LOE->op2 = la;

    Type loT = lo->type;
    if ( loT.is_invalid() ) {
        LOE->type = invalid_type;
        return LOE;
    }

    Type laT = la->type;
    if (laT.is_invalid() ) {
        LOE->type = invalid_type;
        return LOE;
    }

    if ( op == "||" ) {
        if ( laT.isInt() && loT.isInt() ) {
            LOE->type = Type( U_CHAR_T, 0, 0 );
        } else {
            LOE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + loT.get_name() + " and " + laT.get_name(), line_no );
            return LOE;
        }
    } else {
        cerr << "Incorrect logical or expression. Something went wrong\n";
        exit( 0 );
    }

    LOE->add_children( lo, la );
    LOE->res = la->res; 
	append(LOE->truelist,lo->truelist);
    append(LOE->truelist,la->truelist);
    append(LOE->falselist,la->falselist);
    return LOE;
}


Expression *create_conditional_expression( string op, Expression *lo, Expression *te, Expression *coe ) {
    ConditionalExpression *CE = new ConditionalExpression();
    CE->name = "conditional_expression";
    CE->op = op;
    CE->op1 = lo;
    CE->op2 = te;
    CE->op3 = coe;
    
    Type loT = lo->type;
    if ( loT.is_invalid() ) {
        CE->type = invalid_type;
        return CE;
    }

    Type teT = te->type;
    if ( teT.is_invalid() ) {
        CE->type = invalid_type;
        return CE;
    }

    Type coeT = coe->type;
    if ( coeT.is_invalid() ) {
        CE->type = invalid_type;
        return CE;
    }
    

    if ( loT.isInt() ) {
        if ( coeT.isIntorFloat() &&  teT.isIntorFloat() ) {
            if ( teT.typeIndex > coeT.typeIndex ) {
                CE->type = teT;
            } else {
                CE->type = coeT;
            }
            if ( !teT.isUnsigned() ) {
                CE->type.make_signed();
            }
            if ( !coeT.isUnsigned() ) {
                CE->type.make_signed();
            }
        } else if ( teT.typeIndex == coeT.typeIndex && teT.ptr_level == coeT.ptr_level ) {
            CE->type = teT;
        } else {
            CE->type = invalid_type;
            error_message( " Type mismatch in conditional expression for operands of type " + teT.get_name() + " and " + coeT.get_name(), line_no );
            return CE;
        }
    } else {
        CE->type = invalid_type;
        error_message( "Comparison expression is not an int", line_no );
        return CE;
    }
    CE->add_children( lo, te, coe );
    return CE;
}


Expression *create_assignment_expression( Expression *ue, Node *n_op, Expression *ase ) {
    AssignmentExpression *CAE = new AssignmentExpression();
    CAE->name = "assignment_expression";
    Terminal *terminal_op = dynamic_cast<Terminal *>( n_op );
    string op = terminal_op->name;

    CAE->op = op;
    CAE->op1 = ue;
    CAE->op2 = ase;

    Type ueT = ue->type;
    Type aseT = ase->type;
    if ( ueT.is_invalid() ) {
        CAE->type = invalid_type;
        return CAE;
    }
    if ( aseT.is_invalid() ) {
        CAE->type = invalid_type;
        return CAE;
    }

    if ( ueT.is_const ) {
        error_message( "Invalid assignment to constant expression", n_op->line_no, n_op->col_no );
        CAE->type = invalid_type;
        return CAE;
    }

    if ( op == "=" ) {
        if ( ( ueT.isIntorFloat() ) && ( aseT.isIntorFloat() ) ) {
            if ( ueT.typeIndex != aseT.typeIndex ) {
                warning_message( "Operation " + op + " between operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no,  n_op->col_no );
            }
            CAE->type = ueT;
        }
        else if ( ueT.ptr_level > 0 && aseT.ptr_level > 0 ) {
            if ( aseT.typeIndex != VOID_T && ( ueT.typeIndex != aseT.typeIndex || ueT.ptr_level != aseT.ptr_level ) ) {
                warning_message( "Assigning pointer of type " + aseT.get_name() + " to " + ueT.get_name(), n_op->line_no, n_op->col_no );
            }
            CAE->type = ueT;
        } 
        else {
            CAE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            return CAE;
        }
        if ( ue->res->type == MEM && !ueT.isPointer() ) {
            CAE->res = ue->res;
            Address *t1;
            MEM_EMIT( ase, t1 );
            emit( ue->res, "()s", t1, nullptr );
            CAE->res->type = TEMP;
        } else if ( ue->res->type == ID3 ) {
            CAE->res = ue->res;
            Address *t1 = ase->res;
            MEM_EMIT( ase, t1 );
            emit( ue->res, "=", t1, nullptr );
            CAE->res->type = TEMP;
        } else if ( ue->res->type == MEM ) {
            CAE->res = ue->res;
            Address *t1 = ase->res;
            emit( ue->res, "()s", t1, nullptr );
            CAE->res->type = TEMP;
        } else {
            error_message( "lvalue required as left operand of assignment", n_op->line_no, n_op->col_no );
            CAE->type = invalid_type;
            return CAE;
        }
    } 
    else if ( op == "&=" ) {
        if ( ueT.isInt() && aseT.isInt() ) {
            CAE->type = ueT;
            if ( !( ueT.isUnsigned() && aseT.isUnsigned() ) ) {
                CAE->type.make_signed();
            }
        } else {
            CAE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            return CAE;
        }
        if ( ue->res->type == ID3 ) {
            emit( ue->res, op.substr( 0, 1 ), ue->res, ase->res );
            CAE->res = ue->res;
			CAE->res->type = TEMP;
        } else if ( ue->res->type == MEM ) {
            Address *t1 = new_temp();
            emit( t1, "()", ue->res, nullptr );
            emit( t1, op.substr( 0, 1 ), t1, ase->res );
            emit( ue->res, "()s", t1, nullptr );
            CAE->res = t1;
        } else {
            CAE->type = invalid_type;
            error_message( "lvalue required as left operand", n_op->line_no, n_op->col_no );
            return CAE;
        }
    } 
    else if ( op == "|=" ) {
        if ( ueT.isInt() && aseT.isInt() ) {
            CAE->type = ueT;
            if ( !( ueT.isUnsigned() && aseT.isUnsigned() ) ) {
                CAE->type.make_signed();
            }
        } else {
            CAE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            return CAE;
        }
        if ( ue->res->type == ID3 ) {
            emit( ue->res, op.substr( 0, 1 ), ue->res, ase->res );
            CAE->res = ue->res;
			CAE->res->type = TEMP;
        } else if ( ue->res->type == MEM ) {
            Address *t1 = new_temp();
            emit( t1, "()", ue->res, nullptr );
            emit( t1, op.substr( 0, 1 ), t1, ase->res );
            emit( ue->res, "()s", t1, nullptr );
            CAE->res = t1;
        } else {
            CAE->type = invalid_type;
            error_message( "lvalue required as left operand", n_op->line_no, n_op->col_no );
            return CAE;
        }
    }
    else if ( op == "^=" ) {
        if ( ueT.isInt() && aseT.isInt() ) {
            CAE->type = ueT;
            if ( !( ueT.isUnsigned() && aseT.isUnsigned() ) ) {
                CAE->type.make_signed();
            }
        } else {
            CAE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            return CAE;
        }
        if ( ue->res->type == ID3 ) {
            emit( ue->res, op.substr( 0, 1 ), ue->res, ase->res );
            CAE->res = ue->res;
			CAE->res->type = TEMP;
        } else if ( ue->res->type == MEM ) {
            Address *t1 = new_temp();
            emit( t1, "()", ue->res, nullptr );
            emit( t1, op.substr( 0, 1 ), t1, ase->res );
            emit( ue->res, "()s", t1, nullptr );
            CAE->res = t1;
        } else {
            CAE->type = invalid_type;
            error_message( "lvalue required as left operand", n_op->line_no, n_op->col_no );
            return CAE;
        }
    }
    else if ( op == "+=" ) {
        if ( ( ueT.isIntorFloat() ) && ( aseT.isIntorFloat() ) ) {
            CAE->type = ueT;
            if ( ueT.typeIndex != aseT.typeIndex ) {
                warning_message( "Operation " + op + " between operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            }
        } else if ( aseT.isInt() &&  ueT.isPointer() ) {
            CAE->type = ueT;
        } else {
            CAE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            return CAE;
        }
        if ( ue->res->type == ID3 ) {
            emit( ue->res, op.substr( 0, 1 ), ue->res, ase->res );
            CAE->res = ue->res;
			CAE->res->type = TEMP;
        } else if ( ue->res->type == MEM ) {
            Address *t1 = new_temp();
            emit( t1, "()", ue->res, nullptr );
            emit( t1, op.substr( 0, 1 ), t1, ase->res );
            emit( ue->res, "()s", t1, nullptr );
            CAE->res = t1;
        }else {
            CAE->type = invalid_type;
            error_message( "lvalue required as left operand", n_op->line_no, n_op->col_no );
            return CAE;
        }
    } 
    else if ( op == "-=" ) {
        if ( ( ueT.isIntorFloat() ) && ( aseT.isIntorFloat() ) ) {
            CAE->type = ueT;
            if ( ueT.typeIndex != aseT.typeIndex ) {
                warning_message( "Operation " + op + " between operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            }
        } else if ( aseT.isInt() &&  ueT.isPointer() ) {
            CAE->type = ueT;
        } else {
            CAE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            return CAE;
        }
        if ( ue->res->type == ID3 ) {
            emit( ue->res, op.substr( 0, 1 ), ue->res, ase->res );
            CAE->res = ue->res;
			CAE->res->type = TEMP;
        } else if ( ue->res->type == MEM ) {
            Address *t1 = new_temp();
            emit( t1, "()", ue->res, nullptr );
            emit( t1, op.substr( 0, 1 ), t1, ase->res );
            emit( ue->res, "()s", t1, nullptr );
            CAE->res = t1;
        }else {
            CAE->type = invalid_type;
            error_message( "lvalue required as left operand", n_op->line_no, n_op->col_no );
            return CAE;
        }
    } 
    else if ( op == "<<=" ) {
        if ( ueT.isInt() && aseT.isInt() ) {
            CAE->type = ueT;
        } else {
            CAE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            return CAE;
        }
        if ( ue->res->type == ID3 ) {
            emit( ue->res, op.substr( 0, 2 ), ue->res, ase->res );
            CAE->res = ue->res;
			CAE->res->type = TEMP;
        } else if ( ue->res->type == MEM ) {
            Address *t1 = new_temp();
            emit( t1, "()", ue->res, nullptr );
            emit( t1, op.substr( 0, 2 ), t1, ase->res );
            emit( ue->res, "()s", t1, nullptr );
            CAE->res = t1;
        } else {
            CAE->type = invalid_type;
            error_message( "lvalue required as left operand", n_op->line_no, n_op->col_no );
            return CAE;
        }
    }     
    else if ( op == ">>=" ) {
        if ( ueT.isInt() && aseT.isInt() ) {
            CAE->type = ueT;
        } else {
            CAE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            return CAE;
        }
        if ( ue->res->type == ID3 ) {
            emit( ue->res, op.substr( 0, 2 ), ue->res, ase->res );
            CAE->res = ue->res;
			CAE->res->type = TEMP;
        } else if ( ue->res->type == MEM ) {
            Address *t1 = new_temp();
            emit( t1, "()", ue->res, nullptr );
            emit( t1, op.substr( 0, 2 ), t1, ase->res );
            emit( ue->res, "()s", t1, nullptr );
            CAE->res = t1;
        } else {
            CAE->type = invalid_type;
            error_message( "lvalue required as left operand", n_op->line_no, n_op->col_no );
            return CAE;
        }
    }
    else if ( op == "/=" ) {
        if ( ( ueT.isIntorFloat() ) && ( aseT.isIntorFloat() ) ) {
            CAE->type = ueT;
            if ( ueT.typeIndex != aseT.typeIndex ) {
                warning_message( "Operation " + op + " between operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            }
        } else {
            CAE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            return CAE;
        }
        if ( ue->res->type == ID3 ) {
            emit( ue->res, op.substr( 0, 1 ), ue->res, ase->res );
            CAE->res = ue->res;
            ue->res->type = TEMP;
        } else if ( ue->res->type == MEM ) {
            Address *t1 = new_temp();
            emit( t1, "()", ue->res, nullptr );
            emit( t1, op.substr( 0, 1 ), t1, ase->res );
            emit( ue->res, "()s", t1, nullptr );
            CAE->res = t1;
        } else {
            CAE->type = invalid_type;
            error_message( "lvalue required as left operand", n_op->line_no, n_op->col_no );
            return CAE;
        }
    } 
    else if ( op == "*=" ) {
        if ( ( ueT.isIntorFloat() ) && ( aseT.isIntorFloat() ) ) {
            CAE->type = ueT;
            if ( ueT.typeIndex != aseT.typeIndex ) {
                warning_message( "Operation " + op + " between operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            }
        } else {
            CAE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            return CAE;
        }
        if ( ue->res->type == ID3 ) {
            emit( ue->res, op.substr( 0, 1 ), ue->res, ase->res );
            CAE->res = ue->res;
            ue->res->type = TEMP;
        } else if ( ue->res->type == MEM ) {
            Address *t1 = new_temp();
            emit( t1, "()", ue->res, nullptr );
            emit( t1, op.substr( 0, 1 ), t1, ase->res );
            emit( ue->res, "()s", t1, nullptr );
            CAE->res = t1;
        } else {
            CAE->type = invalid_type;
            error_message( "lvalue required as left operand", n_op->line_no, n_op->col_no );
            return CAE;
        }
    }
    else if ( op == "%=" ) {
        if ( ( ueT.isInt() && aseT.isInt() ) ) {
            CAE->type = ueT;
        } else {   
            CAE->type = invalid_type;
            error_message( "Undefined operation " + op + " for operands of type " + ueT.get_name() + " and " + aseT.get_name(), n_op->line_no, n_op->col_no );
            return CAE;
        }
        CAE->res = ue->res;
        emit( CAE->res, op.substr( 0, 1 ), ue->res, ase->res );
    } 
    else {
        cerr << "Incorrect logical or expression. Something went wrong\n";
        cerr << "ERROR at line " << line_no << "\n";
        exit( 0 );
    }
    CAE->add_children( ue, ase );
    return CAE;
}


Expression *create_toplevel_expression( Expression *te, Expression *ase ) {
    TopLevelExpression *TLE = new TopLevelExpression();
    TLE->name = "toplevel_expression";
    TLE->op1 = te;
    TLE->op2 = ase;

    Type teT = te->type;
    Type aseT = ase->type;
    if ( teT.is_invalid() ) {
        TLE->type = invalid_type;
        return TLE;
    }
    
    if ( aseT.is_invalid() ) {
        TLE->type = invalid_type;
        return TLE;
    }

    TLE->add_children( te, ase );
    return TLE;
}

void Constant::negate(){
    value = '-'+value;
    switch(ConstantType.typeIndex){
        case U_CHAR_T: val.uc=-val.uc; break;
        case CHAR_T: val.c = -val.c ; break;
        case U_INT_T: val.ui = -val.ui ; break;
        case INT_T: val.i = -val.i ; break;
        case U_LONG_T: val.ul = -val.ul ; break;
        case LONG_T: val.l = -val.l ; break;
        case FLOAT_T: val.f = -val.f ; break;
        case DOUBLE_T: val.d = -val.d ; break;
    }
}

unsigned stou( string const &str, size_t *idx = 0, int base = 10 ) {
    unsigned long answer = stoul( str, idx, base );
    if ( answer > numeric_limits<unsigned>::max() ) {
        throw out_of_range( "stou" );
    }
    return answer;
}

Constant::Constant( const char *_name, const char *_value, unsigned int _line_no, unsigned int _col_no ) : Terminal( _name, _value, _line_no, _col_no ) {
	memset(&val, 0, sizeof(union data));
    ConstantType = Type( -1, 0, false );

    int len = value.length();

    if ( name == "CONSTANT HEX" || name == "CONSTANT INT" ) {
        int digitend = len;
        int islong = 0;
        int isunsigned = 0;
        int i = 0;
        while( i < len ) {
            if ( value[i] == 'u' ) {
                isunsigned = 1;
                if ( i < digitend ) {
                    digitend = i;
                }
            }
            if ( value[i] == 'U' ) {
                isunsigned = 1;
                if ( i < digitend ) {
                    digitend = i;
                }
            }
            if ( value[i] == 'l' ) {
                islong = 1;
                if ( i < digitend ) {
                    digitend = i;
                }
            }
            if ( value[i] == 'L' ) {
                islong = 1;
                if ( i < digitend ) {
                    digitend = i;
                }
            }
            if ( isunsigned == 1 && islong == 1 ) {
                ConstantType.typeIndex = PrimitiveTypes::U_LONG_T;
                if ( name == "CONSTANT HEX" ) {
                    val.ul = stoul( value, nullptr, 16 );
                } else {
                    val.ul = stoul( value );
                }
                return;
            }
            i++;
        }
        if ( islong == 1 ) {
            ConstantType.typeIndex = PrimitiveTypes::LONG_T;
            if ( name == "CONSTANT HEX" ) {
                val.l = stol( value, nullptr, 16 );
            } else {
                val.l = stol( value );
            }
            return;
        } else if ( isunsigned == 1 ) {
            ConstantType.typeIndex = PrimitiveTypes::U_INT_T;
            if ( name == "CONSTANT HEX" ) {
                val.ui = stou( value, nullptr, 16 );
            } else {
                val.ui = stou( value );
            }
            return;
        } else {
            ConstantType.typeIndex = PrimitiveTypes::INT_T;
            if ( name == "CONSTANT HEX" ) {
                val.i = stoi( value, nullptr, 16 );
            } else {
                val.i = stoi( value );
            }
            return;
        }
        
    } 
    else if ( name == "CONSTANT CHAR" ) {
        if ( value[1] == '\\') {
            ConstantType.typeIndex = PrimitiveTypes::CHAR_T;
            switch (value[2]) {
                case 't' : val.c = '\t'; break;
                case 'r' : val.c = '\r'; break;
                case 'n' : val.c = '\n'; break;
                case '0' : val.c = '\0'; break;
                case '\\' :val.c = '\\'; break;
                default:
                    error_message("Invalid Escape Sequence",line_no,col_no);
            }
            return;
        }
        ConstantType.typeIndex = PrimitiveTypes::CHAR_T;
        val.c = value[1];
    }
    else if ( name == "CONSANT EXP" ) {
        int i;
        while ( i < len ) {
            if ( value[i] == 'f' ) {
                ConstantType.typeIndex = PrimitiveTypes::FLOAT_T;
                val.f = stof( value );
                return;
            }
            if ( value[i] == 'F' ) {
                ConstantType.typeIndex = PrimitiveTypes::FLOAT_T;
                val.f = stof( value );
                return;
            }
            i++;
        }
        ConstantType.typeIndex = PrimitiveTypes::LONG_T;
        val.l = stol( value );
        return;
    } 
    else if ( name == "CONSTANT FLOAT" ) {
        int i;
        while ( i < len ) {
            if ( value[i] == 'f' ) {
                ConstantType.typeIndex = PrimitiveTypes::FLOAT_T;
                val.f = stof( value );
                return;
            }
            if ( value[i] == 'F' ) {
                ConstantType.typeIndex = PrimitiveTypes::FLOAT_T;
                val.f = stof( value );
                return;
            }
            i++;
        }
        ConstantType.typeIndex = PrimitiveTypes::DOUBLE_T;
        val.d = stod( value );
        return;
    } 
}

Constant *create_constant( const char *name, const char *value, unsigned int line_no, unsigned int col_no ) {
    Constant *CC = new Constant( name, value, line_no, col_no );
    return CC;
}
