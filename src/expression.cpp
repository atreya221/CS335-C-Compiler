#include "expression.h"
#include "symtab.h"
#include "y.tab.h"
#include "ast.h"

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <list>
#include <vector>
#include <sstream>
#include <utility>
#include <iterator>

using namespace std;


extern int line_no;
extern int col_no;

//#####################################
// EXPRESSION
//#####################################

Expression *create_primary_identifier( Identifier *id ) {
    PrimaryExpression *P = new PrimaryExpression();
    P->isTerminal = 1;
    P->Ival = id;
    SymTabEntry *ste = local_symbol_table.get_symbol_from_table( id->value );
    if (!ste) {
        ste = global_symbol_table.get_symbol_from_table( id->value );
        if (!ste) {
            cerr << "Undefined symbol " << id->value << " at line " << line_no << "\n";
            exit( 0 );
        }
    }
    P->name = "primary_expression";
    P->add_child(id); 
    P->type = ste->type;
    
    return P;
}


Expression *create_primary_expression( Expression *exp ) {
    PrimaryExpression *P = new PrimaryExpression();
    P->isTerminal = 0;
    P->op1 = exp;
    P->type = exp->type;

    P->name = "primary_expression";
    P->add_child(exp);
    return P;
}
   
Expression *create_postfix_expr_arr( Expression *p_exp, Expression *exp ) {
    PostfixExpression *P = new PostfixExpression();
    P->pe = NULL;
    if ( dynamic_cast<PostfixExpression *>(p_exp) ) {
        P->pe = dynamic_cast<PostfixExpression *>(p_exp);
    }
    P->exp = exp;
    Types peT = defined_types[p_exp->type.typeIndex];

    if (p_exp->type.ptr_level == 1) {
        if ( exp->type.isInt() ) {
            P->type = p_exp->type;
            P->type.ptr_level = 0;
        } else {
            cerr << "ERROR: Array subscript is not an integer at line " << line_no << "\n";
            exit( 0 );
        }
    }

    P->name = "ARRAY ACCESS";
    P->add_children( p_exp, exp );
    return P;
}

Expression *create_postfix_expr_struct( string opr, Expression *exp,
                                        Identifier *id ) {
    PostfixExpression *P = new PostfixExpression();
    Types peT = defined_types[exp->type.typeIndex];
    if ( opr == "->" ) {
        if (exp->type.ptr_level == 1 &&  peT.is_struct ) {
            Type *iType = peT.struct_definition->get_member( id );
            if ( iType != nullptr ) {
                P->type = *iType;  
            } else {
                cerr << "Error: operand not defined in struct at line " << line_no << "\n";
            }
        } else {
            cerr<<"Operand '->' Error  at line " << line_no << "\n";
            exit(0);
        }
    }
    else if ( opr == "." ) {
        
        if ( exp->type.ptr_level == 0 && peT.is_struct ) {
            Type *iType = peT.struct_definition->get_member( id );
            if ( iType != nullptr ) {
                P->type = *iType;
            } else {
                cerr << "Error: operand not defined in struct at line " << line_no << "\n"; 
            }
        } else {
            cerr<<"Operand '.' Error at line " << line_no << "\n";
            exit(0);
        }
    } 

    P->name = opr;
    P->add_children( exp, id );
    return P;
}

Expression *create_postfix_expr_ido( string opr, Expression *p_exp ) {
    PostfixExpression *P = new PostfixExpression();
    if ( dynamic_cast<PostfixExpression *>( p_exp ) ) {
        P->pe = dynamic_cast<PostfixExpression *>( p_exp );
    } 
    else {
        P->pe = nullptr;
    }
    P->op = opr;
    
    if ( opr == "--" || opr == "++" ) {
        if ( p_exp->type.isPointer()==0 && p_exp->type.isInt()==0 && p_exp->type.isFloat()==0 ) {
            cerr << "Postfix operator " << opr
                      << " cannot be applied to type" << p_exp->type.get_name() << "\n";
                      cerr << "ERROR at line " << line_no << "\n";
            exit( 0 ); 
        } 
        else {
            P->type = p_exp->type;
        }
    } 
          
    else {
        cerr << "Parse error";
        cerr << "ERROR at line " << line_no << "\n";
        exit( 0 );
    }
    if ( opr == "--" ){
        P->name = "POST DECREMENT";
    } else {
        P->name = "POST INCREMENT";
    }

    P->add_child( p_exp );
    return P;
}

// Unary Expression
Expression *create_unary_expression_ue( string unary_op, Expression *unary_exp ) {
    UnaryExpression *U = new UnaryExpression();
    U->op = unary_op;
    U->op1 = unary_exp;
    Type ueT = unary_exp->type;
    if ( unary_op == "sizeof" ) {
        U->type.ptr_level = 0;
        U->type.is_const = false;
        U->type.typeIndex = PrimitiveTypes::U_INT_T;
    } else if (unary_op == "++" || unary_op == "--" ) {
        if ( ueT.isInt() || ueT.isFloat() || ueT.isPointer() ) {
            U->type = unary_exp->type;
        } else {
            cerr << "Prefix operator " << unary_exp << " cannot be applied to type " << unary_exp->type.get_name() << " at line " << line_no << "\n";
            delete U;
            exit(0);
        }
    } else {
        cerr << "Error parsing Unary Expression at line " << line_no << "\n";
        exit(0);
    }
    U->name = "unary_expression";
    U->add_child( unary_exp );
    return U;
}

Expression *create_unary_expression_cast( Node *node_oprr, Expression *exp ) {
    UnaryExpression *U = new UnaryExpression();
    Non_Terminal *non_term_op = dynamic_cast<Non_Terminal *>( node_oprr );
    string opr = non_term_op->name;
    U->op = opr;
    U->op1 = exp;
    if ( opr == "*" ) {
        if ( exp->type.ptr_level <= 0 ) {
            delete U;
            cerr << "Error : Invalid dereference of type " << exp->type.get_name() << "\n";
            cerr << "ERROR at line " << line_no << "\n";
            exit(0);        
        } 
        else {
            U->type = exp->type;
            U->type.ptr_level--;
        }        
    } 
    else if ( opr == "+" || opr == "-" || opr == "!") {
        if(exp->type.isIntorFloat()){
            U->type = exp->type;    
        }
        else{
            cerr << "Invalid use of unary operator " << opr << " on type " << exp->type.get_name() << "\n";
            cerr << "ERROR at line " << line_no << "\n";
            exit(0);
        }
    }
    else if ( opr == "&" ) {
        U->type = exp->type;
        U->type.ptr_level++;        
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

    if ( opr == "%" ) {
        if ( exp1T.isInt() && exp2T.isInt() ) {
            P->type = exp2T;
            P->type.make_unsigned();
        } else {
            cerr << "Invalid Operands to binary % having type "
                      << exp1T.get_name() << " and " << exp2T.get_name() << "\n";
            cerr << "ERROR at line " << line_no << "\n";
            exit( 0 );
        }
    }
    else if ( opr == "/" || opr == "*" ) {
        if ( exp1T.isFloat() && exp2T.isFloat() ) {
            P->type = exp1T.typeIndex > exp2T.typeIndex ? exp1T : exp2T;
        }
        else if ( exp1T.isInt() && exp2T.isInt() ) {
            P->type = exp1T.typeIndex > exp2T.typeIndex ? exp1T : exp2T;
            if ( !( exp1T.isUnsigned() && exp2T.isUnsigned() ) ) {
                P->type.make_signed();
            }
        }  
        else if ( ( exp1T.isInt() && exp2T.isFloat() ) || ( exp1T.isFloat() && exp2T.isInt() )) {
            P->type = exp1T.isFloat() ? exp1T : exp2T;
        }
        else {
            cerr << "Undefined operation " << opr
                      << " for operands of type " << exp1T.get_name() << " and "
                      << exp2T.get_name() << "\n";
            cerr << "ERROR at line " << line_no << "\n";
        }
    }

    
    Node *node_oprr = create_non_term( ( opr ).c_str() );
    P->name = "multiplicative expression";
    P->add_children( exp1, node_oprr, exp2 );
    return P;
}

Expression *create_additive_expression( string opr, Expression *exp1,
                                        Expression *exp2 ) {
    AdditiveExpression *P = new AdditiveExpression();
    P->op1 = exp1;
    P->op2 = exp2;
    P->op = opr;
    Type exp1T = exp1->type;
    Type exp2T = exp2->type;
    if ( exp2T.isInt() && exp1T.isInt() ) {
        P->type = exp2T.typeIndex > exp1T.typeIndex ? exp2T : exp1T;
        if ( !( exp2T.isUnsigned() && exp1T.isUnsigned())) {
            P->type.make_signed();
        }
    } 
    else if( (exp2T.isPointer()&& exp1T.isInt()) || (exp1T.isPointer() && exp2T.isInt())){
            P->type = exp2T.isPointer() ? exp2T: exp1T;
    } 
    else if ( ( exp2T.isFloat() && exp1T.isInt() ) || ( exp2T.isInt() && exp1T.isFloat())) {
        P->type = exp2T.isFloat() ? exp2T : exp1T; 
    }
    else if ( exp2T.isFloat() && exp1T.isFloat() ) {
        P->type = exp2T.typeIndex > exp1T.typeIndex ? exp2T : exp1T;
    } 
    else if ( exp2T.isPointer() && ( ( exp2T.isFloat() && exp1T.isInt()) || ( exp2T.isInt() && exp1T.isFloat()))) {
        P->type = exp2T.isFloat() ? exp2T : exp1T;
    } 
    else {
        cerr << "Undefined operation " << opr << " for operands of type " << exp2T.get_name()
                  << " and " << exp1T.get_name() << " at line " << line_no << "\n";
        exit( 0 );
    }

    P->name = "additive_expression";
    Node *node_oprr = create_non_term( ( opr ).c_str() );
    P->add_children( exp1, node_oprr, exp2 );

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
    

    if ( opr == ">>" || opr == "<<" ) {
        if ( exp1T.isInt() == 0 ) {
            if ( exp2T.isInt() == 0 ){
                P->type = exp1T;
            }
        } else {
            cerr << "Undefined operation of " << opr
                      << " on operands of type " << exp2T.get_name() << " and "
                      << exp1T.get_name() << "\n";
            cerr << "ERROR at line " << line_no << "\n";
            exit( 0 );
        }
    } else {
        cerr << "Incorrect shift expression. Something went wrong\n";
        exit( 0 );
    }

    
    Node *node_oprr = create_non_term( ( opr ).c_str() );
    P->name = "shift_expression";
    P->add_children( exp1, node_oprr, exp2 );
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
    
    if ( opr == ">=" || opr == "<=" || opr == "<" || opr == ">"  ) {
        if ( exp1T.isInt() || exp1T.isFloat() ) {
            if( exp2T.isInt() || exp1T.isFloat() ) {
                P->type = Type( U_CHAR_T, 0, 0 );
            }
        } 
        else {
            cerr << "Undefined operation of " << opr
                      << " on operands of type " << exp1T.get_name() << " and "
                      << exp2T.get_name() << "\n";
            cerr << "ERROR at line " << line_no << "\n";
            exit( 0 );
        }
    } 
    else {
        cerr << "Incorrect relation expression. Something went wrong\n";
        exit( 0 );
    }

    Node *node_oprr = create_non_term( ( opr ).c_str() );
    P->name = "relational_expression";
    P->add_children( exp1, node_oprr, exp2 );
    return P;
}

Expression *create_equality_expression( string opr, Expression *exp1,
                                        Expression *exp2 ) {
    EqualityExpression *P = new EqualityExpression();
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;
    
    Type exp1T = exp1->type;
    Type exp2T = exp2->type;

    if ( opr == "==" || opr == "!=" ) {
        if (exp1T.isInt() || exp1T.isFloat()) {
            if( exp2T.isInt() || exp2T.isFloat()){
                P->type = Type( U_CHAR_T, 0, 0 );
            }
        }
        else {
            cerr << "Undefined operation of " << opr << " on operands of type " << exp2T.get_name() << " and " <<  exp1T.get_name() << " at line " << line_no << "\n";
            exit( 0 );
        }
    } 
    else {
        cerr << "Incorrect equality expression. Something went wrong\n";
        exit( 0 );
    }

    Node *node_oprr = create_non_term( ( opr ).c_str() );
    P->name = "eqality_expression";
    P->add_children(exp1, node_oprr, exp2);
    return P;
}

// And Expression
Expression *create_and_expression( string opr, Expression *exp1,
                                   Expression *exp2 ) {
    AndExpression *P = new AndExpression();
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;

    Type exp1T = exp1->type;
    Type exp2T = exp2->type;
    
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
            cerr << "Undefined operation of " << opr
                      << " on operands of type " << exp1T.get_name() << " and "
                      << exp2T.get_name() << "\n";
            cerr << "ERROR at line " << line_no << "\n";
            exit( 0 );
        }      
    } 
    else {        
        cerr << "Incorrect and_expression. Something went wrong\n";
        exit( 0 );
    }
    Node *node_opr = create_non_term( ( opr ).c_str() );
    P->name = "and_expression";
    P->add_children( exp1, node_opr, exp2 );
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
            cerr << "Undefined operation of " << opr
                      << " on operands of type " << exp1T.get_name() << " and "
                      << exp2T.get_name() << "\n";
            cerr << "ERROR at line " << line_no << "\n";
            exit( 0 );
        }
    } 
    else {
        cerr << "Incorrect exclusive or expression. Something went wrong\n";
        exit( 0 );
    }

    Node *node_oprr = create_non_term( ( opr ).c_str() );
    P->name = "exclusive_or_expression";
    P->add_children( exp1, node_oprr, exp2 );
    return P;
}

Expression *create_inclusive_or_expression( string opr, Expression *exp1, Expression *exp2 ) {
    InclusiveorExpression *P = new InclusiveorExpression();
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;
    
    Type exp1T = exp1->type;
    Type exp2T = exp2->type;
    
    if ( opr == "|" ) {
        if ( exp1T.isInt()) {
            if ( exp2T.isInt()){
                P->type = exp1T.typeIndex > exp2T.typeIndex ? exp1T : exp2T;
                if ( !(exp1T.isUnsigned() && exp2T.isUnsigned() )) {
                    P->type.make_signed();
                } else {
                    cerr << "Undefined operation of " << opr << " on operands of type " << exp1T.get_name() << " and " << exp2T.get_name() << " at line " << line_no << "\n";
                    exit( 0 );
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

    if ( opr == "&&" ) {
        if ( exp2T.isIntorFloat() ){
            if( exp1T.isIntorFloat() ) {
                P->type = Type( U_CHAR_T, 0, 0 );
            }
        } 
        else {
            cerr << "Undefined operation of " << opr
                      << " on operands of type " << exp2T.get_name() << " and "
                      << exp1T.get_name() << "\n";
            cerr << "ERROR at line " << line_no << "\n";
            exit( 0 );
        }
    } 
    else {
        cerr << "Incorrect logical and expression. Something went "
                     "wrong\n";
        exit( 0 );
    }

    P->name = "logical_and_expression";
    Node *node_opr = create_non_term( ( opr ).c_str() );
    P->add_children( exp1, node_opr, exp2 );
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
    
    if ( opr == "||" ) {
        if ( exp1T.isIntorFloat() ) {
            if( exp2T.isIntorFloat() ){
                P->type = Type( U_CHAR_T, 0, 0 );
            }
        } else {
            cerr << "Undefined operation of " << opr
                      << " on operands of type " << exp2T.get_name() << " and "
                      << exp1T.get_name() << "\n";
            cerr << "ERROR at line " << line_no << "\n";
            exit( 0 );
        }
    } else {
        cerr << "Incorrect logical or expression. Something went "
                     "wrong\n";
        exit( 0 );
    }

    Node *node_opr = create_non_term( ( opr ).c_str() );
    P->name = "logical_or_expression";
    P->add_children( exp1, node_opr, exp2 );
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
            exit( 0 );
        }
    }

    P->name = "conditional_expression";
    P->add_children( exp1, exp2, exp3 );
    return P;
}

// AssignmentExpression

Expression *create_assignment_expression( Expression *exp1, Node *node_opr,
                                          Expression *exp2 ) {
    AssignmentExpression *P = new AssignmentExpression();
    Non_Terminal *nt_op = dynamic_cast<Non_Terminal *>( node_opr );
    string opr = nt_op->name;
    
    P->op = opr;
    P->op1 = exp1;
    P->op2 = exp2;
    
    Type exp1T = exp1->type;
    Type exp2T = exp2->type;
    if(exp1T.is_const){
        cerr << "ERROR: Cannot assign to constant variable at line " << line_no << "\n";
        exit(0);
    }
    if (opr == "=") {
        if (exp1T.isIntorFloat()){
            if(exp2T.isIntorFloat()) {
                if ( exp1T.typeIndex != exp2T.typeIndex ) {
                    cout << "Warning: operation " << opr << " between " << exp1T.get_name() << " and " << exp2T.get_name() << " on line " << line_no << "\n";
                }
                P->type = exp1T;
            }
        } else if (exp1T.ptr_level > 0 ) {
            if( exp2T.ptr_level > 0 ) {
                if(exp1T.ptr_level == exp2T.ptr_level){
                    if ( exp1T.typeIndex == exp2T.typeIndex ) {
                        P->type = exp1T;
                    }
                    else {
                        cerr << "Undefined operation of " << opr << " on operands of type " << exp1T.get_name() << " and " << exp2T.get_name() << " at line " << line_no << "\n";
                        exit( 0);
                    }
                }
            }
        }
        else{
            cerr << "Undefined operation of " << opr << " on operands of type " << exp1T.get_name() << " and " << exp2T.get_name() << " at line " << line_no << "\n";
            exit(0);
        }
    }    
    else if ( opr == "*=" || opr == "/="  ) {
        if (exp1T.isIntorFloat()){ 
            if(exp2T.isIntorFloat()) {
                if ( exp1T.typeIndex != exp2T.typeIndex ) {
                    cout << "Warning: operation " << opr << " between " << exp1T.get_name() << " and " << exp2T.get_name() << " on line " << line_no << "\n";
                    }
                    P->type = exp1T;
                }
        } else {
            cerr << "Undefined operation of " << opr << " on operands of type " << exp1T.get_name() << " and " << exp2T.get_name() << " at line " << line_no << "\n";
            exit( 0 );
        }
    }
    else if (opr == "+=" || opr =="-="){
        if (exp1T.isIntorFloat()) {
            if(exp2T.isIntorFloat()){
                if ( exp1T.typeIndex != exp2T.typeIndex ) {
                    cout << "Warning: operation " << opr << " between " << exp1T.get_name() << " and " << exp2T.get_name() << " on line " << line_no << "\n";
                }
                P->type = exp1T;
            }
        }
        else if(exp1T.isPointer()){
            if(exp2T.isInt())
                P->type = exp1T;
        }
        else {
            cerr << "Undefined operation of " << opr << " on operands of type " << exp1T.get_name() << " and " << exp2T.get_name() << " at line " << line_no << "\n";
            exit( 0 );
        }
    } else if ( opr == "%=" ) {
        if (exp1T.isInt()) {
                if(exp2T.isInt()) {
                    P->type = exp1T;
            }
        } else {
            cerr << "Invalid Operands " << opr << "having type " << exp1T.get_name()
                      << " and " << exp2T.get_name() << "\n";
            cerr << "ERROR at line " << line_no << "\n";
            exit( 0 );
        }
    } else if ( opr == "<<=" || opr == ">>=" ) {
        if (exp1T.isInt()) {
                if(exp2T.isInt()) {
                    P->type = exp1T;
            }
        } else {
            cerr << "Undefined operation of " << opr
                      << " on operands of type " << exp1T.get_name() << " and "
                      << exp2->name << "\n";
            cerr << "ERROR at line " << line_no << "\n";
            exit( 0 );
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
                cerr << "Undefined operation of " << opr
                        << " on operands of type " << exp1T.get_name() << " and "
                        << exp2T.get_name() << "\n";
                cerr << "ERROR at line " << line_no << "\n";
                exit( 0 );
            }
        }
    } else {
        cerr << "Incorrect logical or expression. Something went wrong\n";
        cerr << "ERROR at line " << line_no << "\n";
        exit( 0 );
    }
    P->name="assignment_expression";
    P->add_children(exp1, exp2);
    return P;
}

// TopLevelExpression

Expression *create_toplevel_expression( Expression *exp1, Expression *exp2 ) {
    TopLevelExpression *P = new TopLevelExpression();
    P->op1 = exp1;
    P->op2 = exp2;
    // Type exp1T = exp1->type;
    // Type exp2T = exp2->type;
    //////Complete it later///////////

    P->name = "toplevel_expression";
    P->add_children( exp1, exp2 );
    return P;
}