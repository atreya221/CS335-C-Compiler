#include "expression.h"
#include "symtab.h"
#include "statement.h"
#include "y.tab.h"
#include "ast.h"

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <list>
#include <vector>
#include <3ac.h>
#include <statement.h>
#include <sstream>
#include <utility>
#include <string>
#include <string.h>
#include <iterator>

using namespace std;


extern int line_no;
extern int col_no;

Type invalid_type;

//#####################################
// EXPRESSION
//#####################################

Expression *create_primary_identifier( Identifier *id ) {
    PrimaryExpression *P = new PrimaryExpression();
    P->isTerminal = 1;
    P->Ival = id;

    P->name = "primary_expression";
    SymTabEntry *ste = local_symbol_table.get_symbol_from_table( id->value );
    if (!ste) {
        ste = global_symbol_table.get_symbol_from_table( id->value );
        if (!ste) {
            error_message( "Undeclared symbol " + id->value, id->line_no, id->col_no );
            P->type = invalid_type;
            return P;
        }
    }
    P->add_child(id); 
    P->type = ste->type;
    
    // P->res = new_3id(ste);
    P->line_no = id->line_no;
    P->col_no = id->col_no;
    return P;
}

   
Expression *create_postfix_expr_arr( Expression *p_exp, Expression *exp ) {
    PostfixExpression *P = new PostfixExpression();
    P->pe = NULL;
    if ( dynamic_cast<PostfixExpression *>(p_exp) ) {
        P->pe = dynamic_cast<PostfixExpression *>(p_exp);
    }
    P->name = "ARRAY ACCESS";
    P->exp = exp;

    if (p_exp->type.is_invalid() || exp->type.is_invalid()) {
        P->type = invalid_type;
        return P;
    }

    if(p_exp->type.is_arr){
        P->type = p_exp->type;
        P->type.ptr_level--;
        P->type.arr_size--;
        P->type.arr_sizes.erase(P->type.arr_sizes.begin());
        P->type.is_const = false;

        //TODO: remove comments
        // Address * t1;
        // if(exp->res->name == CON){
        //     long off = stol(exp->res->name) *P->type.get_size();
        //     t1 = new_3const(off, INT3);
        // }
        // else{
        //     t1 = new_temp();
        //     emit(t1, "*", exp->res, new_3const(P->type.get_size(), INT3));
        // }

        if(P->type.ptr_level == 0){
            P->type.is_pointer = false;
        }
        if(P->type.arr_size == 0){
            P->type.is_arr = 0;
        }

        // P->res = new_mem(P->type);
        // emit(P->res, "+", p_exp->res, t1);
    }
    else if (p_exp->type.is_pointer){
        P->type = p_exp->type;
		P->type.is_const = false;
        P->type.ptr_level--;
		if ( P->type.ptr_level == 0 ) {
			P->type.is_pointer = false;
		}

		// Address * t1;
		// if ( e->res->type == CON ) {
		// 	unsigned long long off = std::stol(e->res->name)*P->type.get_size();
		// 	t1 = new_3const(off, INT3);
		// } else {
		// 	t1 = new_temp();
		// 	emit( t1, "*", e->res, new_3const( P->type.get_size() , INT3));
		// }
		
		// P->res = new_mem(P->type);


		// if (pe->res->type == MEM ) {
		// 	// Dereference the pointer
		// 	Address * t2 = new_mem(pe->type);
		// 	emit(t2, "()", pe->res, nullptr);
		// 	emit( P->res, "+", t2, t1 );

		// } else {
		// 	emit( P->res, "+", pe->res, t1 );
		// }
    }
    else{
        error_message("Value is neither array nor pointer", line_no);
        P->type = invalid_type;
    }


    P->add_children( p_exp, exp );
    P->line_no = p_exp->line_no;
    P->col_no = p_exp->col_no;
    return P;
}

Expression *create_postfix_expr_struct( string opr, Expression *exp,
                                        Identifier *id ) {
    PostfixExpression *P = new PostfixExpression();

    if(exp->type.is_invalid()){
        P->type = invalid_type;
        return P;
    }
    Types peT = defined_types[exp->type.typeIndex];

    if ( opr == "->" ) {
        if (exp->type.ptr_level == 1 &&  (peT.is_struct || peT.is_union) ) {
            if(peT.struct_definition == nullptr){
                error_message(id->value + " is not a member of " + peT.name, id->line_no, id->col_no);
                P->type = invalid_type;
                return P;
            }

            Type *iType = peT.struct_definition->get_member( id );
            if ( iType != nullptr ) {
                P->type = *iType;  
            } else {
                error_message(id->value + " is not a member of " + peT.name, id->line_no, id->col_no);
                P->type = invalid_type;
                return P;
            }
        } else {
            error_message("Invalid operand -> with type " + exp->type.get_name(), id->line_no, id->col_no);
            P->type = invalid_type;
            return P;
        }

        // size_t offset = peT.struct_definition->get_offset(id);
        // if(offset == 0 && pe->res->type != MEM){
        //     P->res = new_mem(P->type);
        //     emit(P->res, "=", exp->res, nullptr);
        // }
        // else if( exp->res->type != MEM && offset != 0){
        //     P->res = new_mem(P->type);
        //     emit(P->res, "+", exp->res, new_3const(peT.struct_definition->get_offset(id), INT3));
        // }
        // else if (offset == 0){
        //     P->res = new_mem(P->type);
		//     emit(P->res, "()", exp->res, nullptr);
        // }
        // else{
            // Address * t1 = new_temp();
            // emit(t1, "()", exp->res, nullptr);
            // P->res = new_mem(P->type);
            // emit(P->res, "+", t1, new_3const( peT.struct_definition->get_offset(id) , INT3 )); 
        // }
    }
    else if ( opr == "." ) {
        
        if ( exp->type.ptr_level == 0 && (peT.is_struct || peT.is_union) ) {
            if(peT.struct_definition == nullptr){
                error_message(id->value + " is not a member of " + exp->type.get_name(), id->line_no, id->col_no);
                P->type = invalid_type;
                return P;
            }

            Type *iType = peT.struct_definition->get_member( id );
            if ( iType != nullptr ) {
                P->type = *iType;
            } else {
                error_message(id->value + " is not a member of " + peT.name, id->line_no, id->col_no);
                P->type = invalid_type;
                return P; 
            }
        } else {
            error_message("Invalid operand . with type " + exp->type.get_name(), id->line_no, id->col_no);
            P->type = invalid_type;
            return P; 
        }
    } 

    // size_t offset = peT.struct_definition->get_offset(id);
    // if(offset == 0){
    //     P->res = new_mem(P->type);
    //     emit(P->res, "=", exp->res, nullptr);
    // }
    // else{
    //     P->res = new_mem(P->type);
    //     emit(P->res, "+", exp->res, new_3const(peT.struct_definition->get_offset(i), INT3));
    // }
    return P;
}


Expression *create_postfix_expr_ido( Terminal * opr, Expression *p_exp ) {
    PostfixExpression *P = new PostfixExpression();
    if ( dynamic_cast<PostfixExpression *>( p_exp ) ) {
        P->pe = dynamic_cast<PostfixExpression *>( p_exp );
    } 
    else {
        P->pe = nullptr;
    }
    

    if ( p_exp->type.is_invalid() ) {
        P->type = invalid_type;
        return P;
    }

    P->op = opr->name;

    if ( opr->name == "++" )
        P->name = "POST INCREMENT";
    else
        P->name = "POST DECREMENT";

    string op_code = opr->name.substr( 0, 1 );

    // Address *inc_value;
    
    if (  opr->name != "++" && opr->name != "--" ) {
		cerr << "PANIC: Invalid operation " << opr->name <<"\n";
		assert(0);
		return P;
    }
	if ( p_exp->type.is_const == true ) {
		error_message( "Invalid operand " + opr->name + " with constant type", opr->line_no, opr->col_no );
		P->type = invalid_type;
		return P;
	}

	if ( p_exp->type.isPointer() ) {
		P->type = p_exp->type;
		// P->res = new_mem(P->type);
		Type t = p_exp->type;
		t.ptr_level--;
		// inc_value = new_3const( t.get_size() , INT3);
	} else if ( p_exp->type.isInt() ) {
		// P->res = new_temp();
		P->type = p_exp->type;
		// inc_value = new_3const( 1, INT3 );
	} else if ( p_exp->type.isFloat() ) {
		// P->res = new_temp();
		P->type = p_exp->type;
		// inc_value = new_3const( 1.0 , FLOAT3 );
	} else {
		// Error postfix operator
		error_message( "Invalid operand " + opr->name + " with type " + p_exp->type.get_name(), opr->line_no, opr->col_no );
		delete P->res;
		P->res = nullptr;
		P->type = invalid_type;
		return P;
	}

    P->add_child( p_exp );

    // if ( p_exp->res->type == MEM ) {
    //     Address *t1 = new_temp();
    //     emit( P->res, "()", p_exp->res, nullptr );
    //     emit( t1, "=", P->res, nullptr);
    //     emit( t1, op_code, t1, inc_value );
    //     emit( p_exp->res, "()s", t1, nullptr );
    // } else if ( p_exp->res->type == ID3 ) {
    //     emit( P->res, "=", p_exp->res, nullptr );
    //     emit( p_exp->res, op_code, p_exp->res, inc_value );
    // } else {
	// 	delete inc_value;
	// 	delete P->res;
	// 	P->res = nullptr;
	// 	inc_value = nullptr;
    //     error_message( "lvalue required as operand to" + opr->name, opr->line_no, opr->col_no + 1 );
    //     P->type = invalid_type;
    //     return P;
    // }
    return P;
}

// Unary Expression
Expression *create_unary_expression_ue( Terminal* unary_opr, Expression *unary_exp ) {
    UnaryExpression *U = new UnaryExpression();
    U->op = unary_opr->name;
    U->op1 = unary_exp;
    Type ueT = unary_exp->type;

    if(ueT.is_invalid()){
        U->type = invalid_type;
        return U;
    }
    string unary_op = unary_opr->name;
    U->name = unary_op;
    // Address *inc_value = nullptr;


    if ( unary_op == "sizeof" ) {
        U->name = "sizeof";
        U->type = Type();
        U->type.typeIndex = PrimitiveTypes::INT_T;
        U->type.ptr_level = 0;
        U->type.is_const = true;
        // U->res = new_3const(unary_exp->type.get_size(), INT3);
    } else if (unary_op == "++" || unary_op == "--" ) {
        if(ueT.is_const){
            error_message("Invalid operand " + unary_op + " with costant type", unary_opr->line_no, unary_opr->col_no);
            U->type = invalid_type;
            return U;
        }

        unary_op = unary_op.substr( 0, 1 );
		if ( unary_exp->type.isPointer() ) {
			U->type = unary_exp->type;
            // U->res = new_mem(U->type);
			Type t = unary_exp->type;
			t.ptr_level--;
			// inc_value = new_3const( t.get_size() , INT3 );
		} else if ( unary_exp->type.isInt() ) {
            // U->res = new_temp();
			U->type = unary_exp->type;
			// inc_value = new_3const( 1 , INT3 );
		} else if ( unary_exp->type.isFloat() ) {
            // U->res = new_temp();
			U->type = unary_exp->type;
			// inc_value = new_3const( 1.0, FLOAT3 );
		} else {
			error_message( "Invalid operand " + unary_op + " with type " + unary_exp->type.get_name(), unary_opr->line_no, unary_opr->col_no );
			delete U->res;
			U->res = nullptr;
			U->type = invalid_type;
			return U;
		}


        // if ( unary_exp->res->type == MEM ) {
        //     Address * t1 = new_temp();
        //     emit( t1, "()", unary_exp->res, nullptr );
        //     emit( U->res, unary_op, t1, inc_value );
        //     emit( unary_exp->res, "()s", U->res, nullptr );
        // } else if ( unary_exp->res->type == ID3 ) {
        //     U->res = unary_exp->res;
		// 	unary_exp->res->type = TEMP;
        //     emit( U->res, unary_op, U->res, inc_value );
        // } else {
		// 	delete inc_value;
		// 	inc_value = nullptr;
        //     error_message( "lvalue required as unary " + unary_opr->name + " operand", unary_opr->line_no, unary_opr->col_no );
        //     U->type = invalid_type;
        //     return U;
        // }

    } else {
        cerr << "Error parsing Unary Expression at line " << line_no << "\n";
        exit(0);
    }
    U->add_child( unary_exp );
    return U;
}

Expression *create_unary_expression_cast( Node *node_oprr, Expression *exp ) {
    UnaryExpression *U = new UnaryExpression();
    Terminal *term_op = dynamic_cast<Terminal *>( node_oprr );
    string opr = term_op->name;
    U->op = opr;
    U->op1 = exp;

    Type expT = exp->type;
    if(expT.is_invalid()){
        U->type = invalid_type;
        return U;
    }

    if ( opr == "*" ) {
        if ( exp->type.ptr_level <= 0 || exp->type.is_arr) {
            error_message("Cannot derefernece type " + expT.get_name(), node_oprr->line_no, node_oprr->col_no);
            U->type = invalid_type;
            return U;
        } 

        U->type = exp->type;
        U->type.ptr_level--;
        if(U->type.ptr_level == 0){
            U->type.is_pointer = false;
        }
        else{
            U->type.is_pointer = true;
        }
        
        // create_new_save_live(false);
        // if ( exp->res->type == MEM  ) {
		// 	U->res = new_mem(U->type);
		// 	emit( U->res, "()", exp->res, nullptr );
		// } else if ( exp->res->type == ID3 ) {
		// 	U->res = new_mem(U->type);
		// 	emit(U->res, "=", exp->res, nullptr );
		// } else {
		// 	error_message( "lvalue required as unary * operand", node_oprr->line_no, node_oprr->col_no );
		// 	U->type = invalid_type;
		// 	return U;
		// }

    } 
    else if ( opr == "+" || opr == "-") {
        if(exp->type.isIntorFloat()){
            U->type = exp->type;
            U->type.make_signed();

            // Address *t1;
            // MEM_EMIT( exp, t1 );
            // U->res = new_temp();
            // Address * zero_value = nullptr;
            // if ( exp->type.isInt() ) {
                // zero_value = new_3const( 0, INT3 );
            // } 
            // else if ( exp->type.isFloat() ) {
                // zero_value = new_3const( 0.0 , FLOAT3 );
            // } else {
                // Should not come here
                // assert(0);
            // }

            // emit( U->res, opr, zero_value, t1 );
                
        }
        else{
            error_message("Invalid operand " + opr + " on type " + expT.get_name(), node_oprr->line_no, node_oprr->col_no);
            U->type = invalid_type;
            return U;
        }
    }
    else if(opr == "!"){
        if (exp->type.isInt()){
            U->type = Type( U_CHAR_T, 0, 0 );
            
            // Address *t1;
            // MEM_EMIT( exp, t1 );
			// U->truelist = exp->falselist;
			// U->falselist = exp->truelist;
            // BACKPATCH ( U );
    		// U->res = t1;
			// U->res = new_temp();
            // emit( U->res, opr, t1, nullptr );
        }
        else{
            error_message( "Invalid operand " + opr + " on type " + expT.get_name(), node_oprr->line_no, node_oprr->col_no );
            U->type = invalid_type;
            return U;
        }
    }
    else if ( opr == "&" ) {
        if(expT.is_func){
            error_message("lvalue required as unary & operand", node_oprr->line_no, node_oprr->col_no);
            U->type = invalid_type;
            return U;
        }    
        U->type = exp->type;
        U->type.ptr_level++;

        U->type.is_pointer = true;
        // if(exp->res->type == MEM){
        //     U->res = new_temp();
        //     emit(U->res, "=", exp->res, nullptr);
        // }
        // else if(exp->res->type == ID3){
        //     U->res = new_temp();
        //     emit(U->res, "la", exp->res, nullptr);
        // }
        // else{
        //     error_message("lvalue required as unary & operand", node_oprr->line_no, node_oprr->col_no);
        //     U->type = invalid_type;
        //     return U;
        // }

    }     
    else {
        cerr << "Parse error, invalid unary operator\n";
        cerr << "ERROR at line " << line_no << "\n";
        exit(0);
    }
    U->name = "unary_expression";
    U->add_children( node_oprr, exp );
    return U;
}

// Multiplicative Expression
Expression *create_multiplicative_expression( string opr, Expression *exp1,
                                              Expression *exp2 ) {
    MultiplicativeExpression *P = new MultiplicativeExpression();
    P->op = opr;
    P->op1 = exp1;
    Type exp1T = exp1->type;

    P->op2 = exp2;
    Type exp2T = exp2->type;

    if(exp1T.is_invalid() || exp2T.is_invalid()){
        P->type = invalid_type;
        return P;
    }

    if ( opr == "%" ) {
        if ( exp1T.isInt() && exp2T.isInt() ) {
            P->type = exp2T;
            P->type.make_unsigned();

            // Address *t1, *t2;
            // MEM_EMIT(exp1, t1);
            // MEM_EMIT(exp2, t2);
            // P->res = new_temp();
            // emit(P->res, opr, t1, t2);

        } else {
            error_message("Invalid operands to " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), line_no);
            P->type = invalid_type;
            return P;
        }
    }
    else if ( opr == "/" || opr == "*" ) {
        if ( exp1T.isFloat() && exp2T.isFloat() ) {
            opr += "f";

            // Address *t1, *t2;
            // MEM_EMIT(exp1, t1);
            // MEM_EMIT(exp2, t2);
            // SAVE_REGS(P, t1, t2);

            if(exp1T.typeIndex < exp2T.typeIndex){
                P->type = exp2T;
            }
            else{
                P->type = exp1T;
            }

            // emit(P->res, opr, t1, t2);

        }
        else if ( exp1T.isInt() && exp2T.isInt() ) {
            if(exp1T.isUnsigned() && exp2T.isUnsigned()){
                opr += "u";
            }
            else if(!exp1T.isUnsigned() && exp2T.isUnsigned()){
                exp1T.make_unsigned();
                opr += "u";
            }
            else if(exp1T.isUnsigned() && !exp2T.isUnsigned()){
                exp2T.make_unsigned();
                opr += "u";
            }
            
            // Address *t1, *t2;
            // MEM_EMIT(exp1, t1);
            // MEM_EMIT(exp2, t2);
            // P->res = new_temp();
            
            if(exp1T.typeIndex < exp2T.typeIndex){
                P->type = exp2T;
            }
            else{
                P->type = exp1T;
            }

            // emit(P->res, opr, t1, t2);
        }  
        else if ( ( exp1T.isInt() && exp2T.isFloat() ) || ( exp1T.isFloat() && exp2T.isInt() )) {
            P->type = exp1T.isFloat() ? exp1T : exp2T;
        }
        else {
            error_message("Undefined operation " + opr + "for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), line_no);
            P->type = invalid_type;
            return P;
        }
    }
    else{
        assert(0);
    }
    
    Node *node_oprr = create_non_term( ( opr ).c_str() );
    P->name = "multiplicative expression";
    P->add_children( exp1, node_oprr, exp2 );
    return P;
}

Expression *create_additive_expression( string opr, Expression *exp1, Expression *exp2 ) {
    AdditiveExpression *P = new AdditiveExpression();
    P->op1 = exp1;
    P->op2 = exp2;
    P->op = opr;
    Type exp1T = exp1->type;
    Type exp2T = exp2->type;

    if(exp1T.is_invalid() || exp2T.is_invalid()){
        P->type = invalid_type;
        return P;
    }

    if ( exp2T.isInt() && exp1T.isInt() ) {
        if(exp1T.isUnsigned() && !exp2T.isUnsigned()){
            exp2T.make_unsigned();
        }
        else if(!exp1T.isUnsigned() && exp2T.isUnsigned()){
            exp1T.make_unsigned();
        }

        // Address *t1, *t2;
        // MEM_EMIT(exp1, t1);
        // MEM_EMIT(exp2, t2);
        // P->res = new_temp();

        if(exp1T.typeIndex < exp2T.typeIndex){
            P->type = exp2T;
        }
        else{
            P->type = exp1T;
        }

        // emit(P->res, opr, t1, t2);

    } 
    else if ((exp1T.isInt() && exp2T.isFloat()) || (exp1T.isFloat() && exp2T.isInt())){
        P->type = exp1T.isFloat() ? exp1T : exp2T;
        //TODO: 3ac
    }
    else if(exp1T.isFloat() && exp2T.isFloat()){
        P->type = exp2T.typeIndex > exp1T.typeIndex ? exp2T : exp1T;
        opr += "f";
    }
    else if(exp1T.isPointer() && exp2T.isInt()){
        P->type = exp1T;
        
        // Address *t1;
        // MEM_EMIT(exp2, t1);
        // P->res = new_mem(P->type);
        // Type t = exp1T;
        // t.ptr_level--;
        // Address * t2 = new_mem();
        // emit(t2, "*", t1, new_3const(t.get_size(), INT3));
        // emit(P->res, opr, exp1->res, t2);

        P->name = "additive_expression";
        Node *node_opr = create_non_term((opr.c_str()));
        P->add_children(exp1, node_opr, exp2);
        return P; 
        
    }
    else if(exp2T.isPointer() && exp1T.isInt()){
        P->type = exp2T;
        
        // Address *t1;
        // MEM_EMIT(exp1, t1);
        // P->res = new_mem(P->type);
        // Type t = exp2T;
        // t.ptr_level--;
        // Address * t2 = new_mem();
        // emit(t2, "*", t1, new_3const(t.get_size(), INT3));
        // emit(P->res, opr, exp2->res, t2);

        P->name = "additive_expression";
        Node *node_opr = create_non_term((opr.c_str()));
        P->add_children(exp1, node_opr, exp2);
        return P; 
        
    }
    else {
        error_message("Undefined operation " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), line_no);
        P->type = invalid_type;
        return P;
    }

    P->name = "additive_expression";
    Node *node_opr = create_non_term( ( opr ).c_str() );
    P->add_children( exp1, node_opr, exp2 );

    return P;
}
// Shift Expression

Expression *create_shift_expression( string opr, Expression *exp1,
                                     Expression *exp2 ) {
    ShiftExpression *P = new ShiftExpression();
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;
    

    Type exp1T = exp1->type;
    Type exp2T = exp2->type;
    
    if(exp1T.is_invalid() || exp2T.is_invalid()){
        P->type = invalid_type;
        return P;
    }

    if ( opr == ">>" || opr == "<<" ) {
        if ( exp1T.isInt() ) {
            if ( exp2T.isInt() ){
                P->type = exp1T;
                if(exp1T.isUnsigned()){
                    opr += "u";
                }
            }
        } else {
            error_message("Undefined operation of " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), line_no);
            P->type = invalid_type;
            return P;
        }
    } else {
        cerr << "Incorrect shift expression. Something went wrong\n";
        exit( 0 );
    }

    
    Node *node_oprr = create_non_term( ( opr ).c_str() );
    P->name = "shift_expression";
    P->add_children( exp1, node_oprr, exp2 );

    // Address *t1, *t2;
    // MEM_EMIT(exp1, t1);
    // MEM_EMIT(exp2, t2);
    // P->res = new_temp();
    // emit(P->res, opr, t1, t2);

    return P;
}

// Relation Expression

Expression *create_relational_expression( string opr, Expression *exp1,
                                          Expression *exp2 ) {
    RelationalExpression *P = new RelationalExpression();
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;
    

    Type exp1T = exp1->type;
    Type exp2T = exp2->type;

    if(exp1T.is_invalid() || exp2T.is_invalid()){
        P->type = invalid_type;
        return P;
    }
    
    if ( opr == ">=" || opr == "<=" || opr == "<" || opr == ">"  ) {
        if ( exp1T.isInt() || exp1T.isFloat() ) {
            if( exp2T.isInt() || exp1T.isFloat() ) {
                P->type = Type( U_CHAR_T, 0, 0 );
                if(exp1T.isUnsigned() != exp2T.isUnsigned()){
                    warning_message("Comparison " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), line_no);
                }
            }
        } 
        else {
            error_message("Undefined operation " + opr + "for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), line_no);
            P->type = invalid_type;
            return P;
        }
    } 
    else {
        cerr << "Incorrect relation expression. Something went wrong\n";
        exit( 0 );
    }

    Node *node_oprr = create_non_term( ( opr ).c_str() );
    P->name = "relational_expression";
    P->add_children( exp1, node_oprr, exp2 );

    // Address *t1, *t2;
    // MEM_EMIT(exp1, t1);
    // MEM_EMIT(exp2, t2);
    // P->res = new_temp();
    // emit(P->res, opr, t1, t2);

    return P;
}

Expression *create_equality_expression( string opr, Expression *exp1, Expression *exp2 ) {
    EqualityExpression *P = new EqualityExpression();
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;
    
    Type exp1T = exp1->type;
    Type exp2T = exp2->type;

    if(exp1T.is_invalid() || exp2T.is_invalid()){
        P->type = invalid_type;
        return P;
    }

    if ( opr == "==" || opr == "!=" ) {
        if (exp1T.ptr_level > 0 && exp2T.ptr_level > 0) {
            P->type = Type(U_CHAR_T, 0, 0);
        }
        else if(exp1T.ptr_level == 0 && exp2T.ptr_level == 0){
            if(exp1T.isInt() || exp1T.isFloat()){
                if(exp2T.isInt() || exp2T.isFloat()){
                    P->type = Type(U_CHAR_T, 0, 0);
                    if(exp1T.isUnsigned() != exp2T.isUnsigned()){
                        warning_message("Comparison " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), line_no);
                    }
                }
            }
        }
        else {
            error_message("Undefined operation " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), line_no);
            P->type = invalid_type;
            return P;
        }
    } 
    else {
        cerr << "Incorrect equality expression. Something went wrong\n";
        exit( 0 );
    }

    Node *node_oprr = create_non_term( ( opr ).c_str() );
    P->name = "eqality_expression";
    P->add_children(exp1, node_oprr, exp2);

    // Address *t1, *t2;
    // MEM_EMIT(exp1, t1);
    // MEM_EMIT(exp2, t2);
    // P->res = new_temp();
    // emit(P->res, opr, t1, t2);

    return P;
}

// And Expression
Expression *create_and_expression( string opr, Expression *exp1, Expression *exp2 ) {
    AndExpression *P = new AndExpression();
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;

    Type exp1T = exp1->type;
    Type exp2T = exp2->type;

    if(exp1T.is_invalid() || exp2T.is_invalid()){
        P->type = invalid_type;
        return P;
    }
    
    if ( opr == "&" ) {
        if ( exp1T.isInt() ) {
            if( exp2T.isInt() ){
                P->type = exp1T.typeIndex > exp2T.typeIndex ? exp1T : exp2T;
                if ( !( exp1T.isUnsigned() && exp2T.isUnsigned() ) ) {
                    P->type.make_signed();
                }
            }
        } 
        else {
            error_message("Undefined operation of " + opr + " on operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), line_no);
            P->type = invalid_type;
            return P;
        }      
    } 
    else {        
        cerr << "Incorrect and_expression. Something went wrong\n";
        exit( 0 );
    }
    Node *node_opr = create_non_term( ( opr ).c_str() );
    P->name = "and_expression";
    P->add_children( exp1, node_opr, exp2 );

    // Address *t1, *t2;
    // MEM_EMIT(exp1, t1);
    // MEM_EMIT(exp2, t2);
    // P->res = new_temp();
    // emit(P->res, opr, t1, t2);

    return P;
}

Expression *create_exclusive_or_expression( string opr, Expression *exp1,
                                            Expression *exp2 ) {
    ExclusiveorExpression *P = new ExclusiveorExpression();
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;
    
    Type exp1T = exp1->type;
    Type exp2T = exp2->type;
    
    if(exp1T.is_invalid() || exp2T.is_invalid()){
        P->type = invalid_type;
        return P;
    }

    if ( opr == "^" ) {
        if ( exp1T.isInt()){
            if ( exp2T.isInt()) {
                P->type = exp2T.typeIndex > exp1T.typeIndex ? exp2T : exp1T;
                if ( !( exp2T.isUnsigned() && exp1T.isUnsigned() ) ) {
                    P->type.make_signed();
                }
            }
        } 
        else {
            error_message("Undefined operation " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), line_no);
            P->type = invalid_type;
            return P;
        }
    } 
    else {
        cerr << "Incorrect exclusive or expression. Something went wrong\n";
        exit( 0 );
    }

    Node *node_oprr = create_non_term( ( opr ).c_str() );
    P->name = "exclusive_or_expression";
    P->add_children( exp1, node_oprr, exp2 );

    // Address *t1, *t2;
    // MEM_EMIT(exp1, t1);
    // MEM_EMIT(exp2, t2);
    // P->res = new_temp();
    // emit(P->res, opr, t1, t2);

    return P;
}

Expression *create_inclusive_or_expression( string opr, Expression *exp1, Expression *exp2 ) {
    InclusiveorExpression *P = new InclusiveorExpression();
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;
    
    Type exp1T = exp1->type;
    Type exp2T = exp2->type;

    if(exp1T.is_invalid() || exp2T.is_invalid()){
        P->type = invalid_type;
        return P;
    }
    
    if ( opr == "|" ) {
        if ( exp1T.isInt()) {
            if ( exp2T.isInt()){
                P->type = exp1T.typeIndex > exp2T.typeIndex ? exp1T : exp2T;
                if ( !(exp1T.isUnsigned() && exp2T.isUnsigned() )) {
                    P->type.make_signed();
                } else {
                    error_message("Undefined operation " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), line_no);
                    P->type = invalid_type;
                    return P;
                }
            }
        } else {
           cerr << "Incorrect inclusive or expression. Something went wrong\n";
           exit( 0 );
        }
    }

    Node *node_oprr = create_non_term( ( opr ).c_str() );
    P->name = "inclusive_or_expression";
    P->add_children( exp1, node_oprr, exp2 );

    // Address *t1, *t2;
    // MEM_EMIT(exp1, t1);
    // MEM_EMIT(exp2, t2);
    // P->res = new_temp();
    // emit(P->res, opr, t1, t2);

    return P;
}

// Logical And
Expression *create_logical_and_expression( string opr, Expression *exp1,
                                           Expression *exp2 ) {
    Logical_andExpression *P = new Logical_andExpression();
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;
    
    Type exp1T = exp1->type;
    Type exp2T = exp2->type;

    if(exp1T.is_invalid() || exp2T.is_invalid()){
        P->type = invalid_type;
        return P;
    }

    if ( opr == "&&" ) {
        if ( exp2T.isIntorFloat() ){
            if( exp1T.isIntorFloat() ) {
                P->type = Type( U_CHAR_T, 0, 0 );
            }
        } 
        else {
            error_message("Undefined operation " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), line_no);
            P->type = invalid_type;
            return P;
        }
    } 
    else {
        cerr << "Incorrect logical and expression. Something went wrong\n";
        exit( 0 );
    }

    P->name = "logical_and_expression";
    Node *node_opr = create_non_term( ( opr ).c_str() );
    P->add_children( exp1, node_opr, exp2 );
    
    // P->res = exp2->res;
    // Address *t1, *t2;

    // append(P->truelist, exp2->truelist);
    // append(P->falselist, exp1->falselist);
    // append(P->falselist, exp2->falselist);

    return P;

}

// Logical or
Expression *create_logical_or_expression( string opr, Expression *exp1,
                                          Expression *exp2 ) {
    Logical_orExpression *P = new Logical_orExpression();
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;

    Type exp1T = exp1->type;
    Type exp2T = exp2->type;

    if(exp1T.is_invalid() || exp2T.is_invalid()){
        P->type = invalid_type;
        return P;
    }
    
    if ( opr == "||" ) {
        if ( exp1T.isIntorFloat() ) {
            if( exp2T.isIntorFloat() ){
                P->type = Type( U_CHAR_T, 0, 0 );
            }
        } else {
            error_message("Undefined operation " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), line_no);
            P->type = invalid_type;
            return P;
        }
    } else {
        cerr << "Incorrect logical or expression. Something went wrong\n";
        exit( 0 );
    }

    Node *node_opr = create_non_term( ( opr ).c_str() );
    P->name = "logical_or_expression";
    //TODO: note here the middle term
    P->add_children( exp1, node_opr, exp2 );

    // P->res = exp2->res;
    // Address *t1, *t2;
    // append(P->truelist, exp1->truelist);
    // append(P->truelist, exp2->truelist);
    // append(P->falselist, exp2->falselist);

    return P;
}

// Conditional

Expression *create_conditional_expression( string opr, Expression *exp1, Expression *exp2, Expression *exp3 ) {
    ConditionalExpression *P = new ConditionalExpression();
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;
    P->op3 = exp3;
    Type exp1T = exp1->type;
    Type exp2T = exp2->type;
    Type exp3T = exp3->type;

    if(exp1T.is_invalid() || exp2T.is_invalid() || exp3T.is_invalid()){
        P->type = invalid_type;
        return P;
    }

    if ( exp1T.isInt() ) {
        if ( exp2T.isIntorFloat() ){
            if( exp3T.isIntorFloat() ) {
                P->type = exp2T.typeIndex > exp3T.typeIndex ? exp2T : exp3T;
                if ( !( exp2T.isUnsigned() && exp3T.isUnsigned() ) ) {
                    P->type.make_signed();
                }
            }
        } else if (exp2T.typeIndex == exp3T.typeIndex) {
            if(exp2T.ptr_level == exp3T.ptr_level)
                P->type = exp2T;
        } else {
            error_message("Type mismatch in conditional expression for operands of type " + exp2T.get_name() + " and " + exp3T.get_name(), line_no);
            P->type = invalid_type;
            return P;

        }
    }
    else{
        error_message("Comparison expression is not an int", line_no);
        P->type = invalid_type;
        return P;
    }

    P->name = "conditional_expression";
    P->add_children( exp1, exp2, exp3 );

    //TODO: add 3ac

    return P;
}

// AssignmentExpression

Expression *create_assignment_expression( Expression *exp1, Node *node_opr,
                                          Expression *exp2 ) {
    AssignmentExpression *P = new AssignmentExpression();
    Terminal *t_op = dynamic_cast<Terminal *>( node_opr );
    string opr = t_op->name;
    
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;
    
    P->name="assignment_expression";
    Type exp1T = exp1->type;
    Type exp2T = exp2->type;

    if(exp1T.is_invalid() || exp2T.is_invalid()){
        P->type = invalid_type;
        return P;
    }

    if(exp1T.is_const){
        error_message("Invalid assignment to constant expression", node_opr->line_no, node_opr->col_no);
        P->type = invalid_type;
        return P;
    }
    if (opr == "=") {
        if (exp1T.isIntorFloat()){
            if(exp2T.isIntorFloat()) {
                if ( exp1T.typeIndex != exp2T.typeIndex ) {
                    warning_message("Operation " + opr + " between operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), node_opr->line_no, node_opr->col_no);
                }
                P->type = exp1T;
            }
        } else if (exp1T.ptr_level > 0 ) {
            if( exp2T.ptr_level > 0 ) {
                if (exp2T.typeIndex != VOID_T && (exp1T.typeIndex != exp2T.typeIndex || exp1T.ptr_level != exp2T.ptr_level)){
                    warning_message("Assigning pointer of type " + exp2T.get_name() + " to " + exp1T.get_name(), node_opr->line_no, node_opr->col_no);
                }
                P->type = exp1T;
            
            }
        }
        else{
            error_message("Undefined operation " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), node_opr->line_no, node_opr->col_no);
            P->type = invalid_type;
            return P;
        }

        // if(exp1->res->type == MEM && !exp1T.isPointer()){
        //     P->res = exp1->res;
        //     Address * t1;
        //     MEM_EMIT(exp2, t1);
        //     emit(exp1->res, "()s", t1, nullptr);
        //     P->res->type = TEMP;
        // }
        // else if(exp1->res->type == MEM){
        //     P->res = exp1->res;
        //     Address *t1 = exp2->res;
        //     emit(exp1->res, "()s", t1, nullptr);
        //     P->res->type = TEMP;
        // }
        // else if(exp1->res->type == ID3){
        //     P->res = exp1->res;
        //     Address *t1 = exp2->res;
        //     MEM_EMIT(exp2, t1);
        //     emit(exp1->res, "=", t1, nullptr);
        //     P->res->type = TEMP;
        // }
        // else{
        //     error_message("lvalue required as left operand of assignment", node_opr->line_no, node_opr->col_no);
        //     P->type = invalid_type;
        //     return P;
        // }
    }    
    else if ( opr == "*=" || opr == "/="  ) {
        if (exp1T.isIntorFloat()){ 
            if(exp2T.isIntorFloat()) {
                if ( exp1T.typeIndex != exp2T.typeIndex ) {
                    warning_message("Operation " + opr + " between operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), node_opr->line_no, node_opr->col_no);    
                }
                P->type = exp1T;
                }
        } else {
            error_message("Undefined operation " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), node_opr->line_no, node_opr->col_no);
            P->type = invalid_type;
            return P;
        }

        // if(exp1->res->type == MEM){
        //     Address *t1 = new_temp();
        //     emit(t1, "()", exp1->res, nullptr);
        //     emit(t1, opr.substr(0, 1), t1, exp2->res);
        //     emit(exp1->res, "()s", t1, nullptr);
        //     P->res = t1;
        // }
        // else if (exp1->res->type == ID3){
        //     emit(exp1->res, opr.substr(0,1), exp1->res, exp2->res);
        //     P->res = exp1->res;
        //     exp1->res->type = TEMP;
        // }
        // else{
        //     error_message("lvalue required as left operand", node_opr->line_no, node_opr->col_no);
        //     P->type = invalid_type;
        //     return P;
        // }
    }
    else if (opr == "+=" || opr =="-="){
        if (exp1T.isIntorFloat()) {
            if(exp2T.isIntorFloat()){
                if ( exp1T.typeIndex != exp2T.typeIndex ) {
                    warning_message("Operation " + opr + " between operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), node_opr->line_no, node_opr->col_no);
                }
                P->type = exp1T;
            }
        }
        else if(exp1T.isPointer()){
            if(exp2T.isInt())
                P->type = exp1T;
        }
        else {
            error_message("Undefined operation " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), node_opr->line_no, node_opr->col_no);
            P->type = invalid_type;
            return P;
        }

        // if(exp1->res->type == MEM){
        //     Address *t1 = new_temp();
        //     emit(t1, "()", exp1->res, nullptr);
        //     emit(t1, opr.substr(0,1), t1, exp2->res);
        //     emit(exp1->res, "()s", t1, nullptr);
        //     P->res = t1;
        // }
        // else if (exp1->res->type == ID3){
        //     emit(exp1->res, opr.substr(0,1), exp1->res, exp2->res);
        //     P->res = exp1->res;
        //     P->res->type = TEMP;
        // }
        // else{
        //     error_message("lvalue required as left operand", node_opr->line_no, node_opr->col_no);
        //     P->type = invalid_type;
        //     return P;
        // }

    } else if ( opr == "%=" ) {
        if (exp1T.isInt()) {
                if(exp2T.isInt()) {
                    P->type = exp1T;
            }
        } else {
            error_message("Undefined operation " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), node_opr->line_no, node_opr->col_no);
            P->type = invalid_type;
            return P;
        }

        // P->res = exp1->res;
        // emit(P->res, opr.substr(0,1), exp1->res, exp2->res);

    } else if ( opr == "<<=" || opr == ">>=" ) {
        if (exp1T.isInt()) {
                if(exp2T.isInt()) {
                    P->type = exp1T;
            }
        } else {
            error_message("Undefined operation " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), node_opr->line_no, node_opr->col_no);
            P->type = invalid_type;
            return P;
        }
    } else if (opr == "&=" || opr == "|=") {
        if( opr == "^=" ) {
            if ( exp1T.isInt() ) {
                if( exp2T.isInt() ) {
                    P->type = exp1T;
                    if ( !( exp1T.isUnsigned() && exp2T.isUnsigned() ) ) {
                        P->type.make_signed();
                    }
                }
            } else {
                error_message("Undefined operation " + opr + " for operands of type " + exp1T.get_name() + " and " + exp2T.get_name(), node_opr->line_no, node_opr->col_no);
            P->type = invalid_type;
            return P;
            }

            // if ( exp1->res->type == MEM ) {
            //     Address *t1 = new_temp();
            //     emit( t1, "()", exp1->res, nullptr );
            //     emit( t1, opr.substr( 0, 1 ), t1, exp2->res );
            //     emit( exp1->res, "()s", t1, nullptr );
            //     P->res = t1;
            // } else if ( exp1->res->type == ID3 ) {
            //     emit( exp1->res, opr.substr( 0, 1 ), exp1->res, exp2->res );
            //     P->res = exp1->res;
            //     P->res->type = TEMP;
            // } else {
            //     error_message("lvalue required as left operand", node_opr->line_no, node_opr->col_no);
            //     P->type = invalid_type;
            //     return P;
            // }

        }
    } else {
        cerr << "Incorrect logical or expression. Something went wrong at line " << line_no << "\n";
        exit( 0 );
    }
    P->add_children(exp1, exp2);
    return P;
}

// TopLevelExpression

Expression *create_toplevel_expression( Expression *exp1, Expression *exp2 ) {
    TopLevelExpression *P = new TopLevelExpression();
    P->op1 = exp1;
    P->op2 = exp2;
    Type exp1T = exp1->type;
    Type exp2T = exp2->type;

    if(exp1T.is_invalid() || exp2T.is_invalid()){
        P->type = invalid_type;
        return P;
    }

    P->name = "toplevel_expression";
    P->add_children( exp1, exp2 );
    return P;
}


unsigned stou(string const &str, size_t *idx = 0, int base = 10){
    unsigned long result = stoul(str, idx, base);
    if(result > numeric_limits<unsigned>::max()){
        throw out_of_range("stou");
    }
    return result;
}


void Constant::negate(){
    value = '-' + value;
    switch (ConstantType.typeIndex)
    {
    case U_CHAR_T:
        val.uc = -val.uc;
        break;
    case CHAR_T:
        val.c = -val.c;
        break;
    case U_INT_T:
        val.ui = -val.ui;
        break;
    case INT_T:
        val.i = -val.i;
        break;
    case U_LONG_T:
        val.ul = -val.ul;
        break;
    case LONG_T:
        val.l = -val.l;
        break;
    case FLOAT_T:
        val.f = -val.f;
        break;
    case DOUBLE_T:
        val.d = -val.d;
        break;
    default:
        break;
    }
}


Constant::Constant(const char * _name, const char * _value, int line_no, int col_no) : Terminal(_name, _value, line_no, col_no) {
    memset(&val, 0, sizeof(union data));
    ConstantType = Type(-1, 0, false);
    int length = value.length();
    if(name == "CONSTANT HEX" || name == "CONSTANT INT"){
        int isLong = 0;
        int isUnsigned = 0;
        int digitEnd = length;
        int i = 0;
        while(i < length){
            if(value[i] == 'l'  || value[i] == 'L'){
                isLong = 1;
                digitEnd = i < digitEnd ? i : digitEnd;
            }
            
            if(value[i] == 'u'  || value[i] == 'U'){
                isUnsigned = 1;
                digitEnd = i < digitEnd ? i : digitEnd;
            }

            if(isLong && isUnsigned){
                ConstantType.typeIndex = PrimitiveTypes::U_LONG_T;
                if(name == "CONSTANT HEX"){
                    val.ul = stoul(value, nullptr, 16);
                }
                else{
                    val.ul = stoul(value);
                }
                return;
            }

            i++;
        }

        if(isLong){
            ConstantType.typeIndex = PrimitiveTypes::LONG_T;
            if(name == "CONSTANT HEX"){
                val.l = stol(value, nullptr, 16);
            }
            else{
                val.l = stol(value);
            }
            return;
        }
        else if(isUnsigned){
            ConstantType.typeIndex = PrimitiveTypes::U_INT_T;
            if(name == "CONSTANT HEX"){
                val.ui = stou(value, nullptr, 16);
            }
            else{
                val.ui = stou(value);
            }
            return;
        }
        else{
            ConstantType.typeIndex = PrimitiveTypes::INT_T;
            if(name == "CONSTANT HEX"){
                val.i = stoi(value, nullptr, 16);
            }
            else{
                val.i = stoi(value); 
            }
            return;
        }
    }
    else if(name == "CONSTANT FLOAT"){
        int i = 0;
        while(i < length){
            if(value[i] == 'f' || value[i] == 'F'){
                ConstantType.typeIndex = PrimitiveTypes::FLOAT_T;
                val.f = stof(value);
                return;
            }
            i++;
        }
        ConstantType.typeIndex = PrimitiveTypes::DOUBLE_T;
        val.d = stod(value);
        return;
    }
    else if(name == "CONSTANT EXP"){
        int i = 0;
        while(i < length){
            if(value[i] == 'f' || value[i] == 'F'){
                ConstantType.typeIndex = PrimitiveTypes::FLOAT_T;
                val.f = stof(value);
                return;
            }
            i++;
        }
        ConstantType.typeIndex = PrimitiveTypes::LONG_T;
        val.l = stol(value);
        return;
    }
    else if(name == "CONSTANT CHAR"){
        if(value[1] == '\\'){
            switch(value[2]){
                case 'n' : val.c = '\n'; break;
                case 'r' : val.c = '\r'; break;
                case 't' : val.c = '\t'; break;
                case '\\' :val.c = '\\'; break;
                case '0' : val.c = '\0'; break;
                default:
                    error_message("Invalid escape sequence", line_no, col_no);
            }
            ConstantType.typeIndex = PrimitiveTypes::CHAR_T;
            return;
        }

        ConstantType.typeIndex = PrimitiveTypes::CHAR_T;
        val.c = value[1];
    }
    
}

Constant *create_constant(const char * name, const char * value, int line_no, int col_no){
    Constant *constant = new Constant(name, value, line_no, col_no); 
    return constant;
}