#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#include "ast.h"
#include "symtab.h"
#include <string>
#include <deque>
#include <map>

using namespace std;


class StructDefinition;
class ParameterTypeList;
class CastExpression;
class AssignmentExpression;
class Constant;
class StringLiteral;
class TopLevelExpression;

class Expression : public Non_Terminal {
  public:
    Type type;
    int num_opearands;
    Expression() : Non_Terminal( "" ){};
};
class PrimaryExpression : public Expression {
  public:
    int isTerminal;
    Expression *op1;
    Identifier *Ival;
    Constant *Cval;
    StringLiteral *Sval;
    
    PrimaryExpression() {
        isTerminal = 0;
        op1 = nullptr;
        Ival = nullptr;
        Cval = nullptr;
        Sval = nullptr;
    }
};

Expression *create_primary_expression( Expression *exp );
Expression *create_primary_identifier( Identifier *id );

class ArgumentExprList : public Expression {
  public:
    Expression *op1; 
    Expression *op2;

    ArgumentExprList() {
        op1 = nullptr;
        op2 = nullptr;
    };
};


class UnaryExpression : public Expression {
  public:
    string op;
    Expression *op1;

    UnaryExpression() { 
        op = "";
        op1 = nullptr;
    }
};

Expression *create_unary_expression_cast( Node *node_op, Expression *exp );
Expression *create_unary_expression_ue( string unary_op,Expression *unary_exp ); 

class CastExpression : public Expression {
  public:
    int typeCast;
    Expression *op1;

    CastExpression() {
        typeCast = -1;
        op1 = nullptr;
    };
};

class AdditiveExpression : public Expression {
  public:
    string op;
    Expression *op1;
    Expression *op2;

    AdditiveExpression() {
        op = "";
        op1 = nullptr;
        op2 = nullptr;
    };
};
Expression *create_additive_expression( string opr, Expression *exp1, Expression *exp2 );

class MultiplicativeExpression : public Expression {
  public:
    string op;
    Expression *op1;
    Expression *op2;

    MultiplicativeExpression() {
        op = "";
        op1 = nullptr;
        op2 = nullptr;
    };
};
Expression *create_multiplicative_expression( string opr, Expression *exp1, Expression *exp2 );

class RelationalExpression : public Expression {
  public:
    string op;
    Expression *op1;
    Expression *op2;

    RelationalExpression() {
        op = "";
        op1 = nullptr;
        op2 = nullptr;
    };
};
Expression *create_relational_expression( string opr, Expression *exp1, Expression *exp2 );

class ShiftExpression : public Expression {
  public:
    string op;
    Expression *op1;
    Expression *op2;

    ShiftExpression() {
        op = "";
        op1 = nullptr;
        op2 = nullptr;
    };
};
Expression *create_shift_expression( string opr, Expression *exp1, Expression *exp2 );

class EqualityExpression : public Expression {
  public:
    string op;
    Expression *op1;
    Expression *op2;

    EqualityExpression() {
        op = "";
        op1 = nullptr;
        op2 = nullptr;
    };
};
Expression *create_equality_expression( string opr, Expression *exp1, Expression *exp2 );


class AndExpression : public Expression {
  public:
    string op;
    Expression *op1;
    Expression *op2;

    AndExpression() {
        op = "";
        op1 = nullptr;
        op2 = nullptr;
    };
};
Expression *create_and_expression( string opr, Expression *exp1, Expression *exp2 );


class ExclusiveorExpression : public Expression {
  public:
    string op;
    Expression *op1;
    Expression *op2;

    ExclusiveorExpression() {
        op = "";
        op1 = nullptr;
        op2 = nullptr;
    };
};
Expression *create_exclusive_or_expression( string opr, Expression *exp1, Expression *exp2 );


class InclusiveorExpression : public Expression {
  public:
    string op;
    Expression *op1;
    Expression *op2;

    InclusiveorExpression() {
        op = "";
        op1 = nullptr;
        op2 = nullptr;
    }
};
Expression *create_inclusive_or_expression( string opr, Expression *exp1, Expression *exp2 );

class Logical_andExpression : public Expression {
  public:
    string op;
    Expression *op1;
    Expression *op2;

    Logical_andExpression() {
        op = "";
        op1 = nullptr;
        op2 = nullptr;
    };
};
Expression *create_logical_and_expression( string opr, Expression *exp1, Expression *exp2 );


class Logical_orExpression : public Expression {
  public:
    string op;
    Expression *op1;
    Expression *op2;

    Logical_orExpression() {
        op = "";
        op1 = nullptr;
        op2 = nullptr;
    };
};
Expression *create_logical_or_expression( string opr, Expression *exp1, Expression *exp2 );


class ConditionalExpression : public Expression {
  public:
    string op;
    Expression *op1;
    Expression *op2;
    Expression *op3;

    ConditionalExpression() {
        op = "";
        op1 = nullptr;
        op2 = nullptr;
        op3 = nullptr;
    };
};
Expression *create_conditional_expression( string opr, Expression *exp1, Expression *exp2, Expression *exp3 );


class AssignmentExpression : public Expression {
  public:
    string op;
    Expression *op1;
    Expression *op2;

    AssignmentExpression() {
        op = "";
        op1 = nullptr;
        op2 = nullptr;
    }
};

Expression *create_assignment_expression(Expression *exp1, Node *node_opr, Expression *exp2 ); 

class TopLevelExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;

    TopLevelExpression() {
        op1 = nullptr;
        op2 = nullptr;
    };
};


class ConstantExpression : public Expression {
  public:
    Expression *op1;
};


class PostfixExpression : public Expression {
  public:
    PostfixExpression *pe;
    string op;
    Identifier *id;
    Expression *exp;
    ArgumentExprList *ae_list;

    PostfixExpression() {
        pe = nullptr;
        op = "";
        id = nullptr;
        exp = nullptr;
        ae_list = nullptr;
    };
};

class Constant : public Terminal {
  public:
    Constant( const char *name );

    Type getConstantType() {
        Type returnT( 2, 0, false );
        int len = value.length();
        if ( name == "CONSTANT INT"  || name == "CONSTANT HEX" ) {        
            int type_long = 0, type_unsigned = 0, i = 0;
            while(i<len){
                if ( value[i] == 'u' )
                    type_unsigned = 1;
                if ( value[i] == 'U' )
                    type_unsigned = 1;
                if ( value[i] == 'l' ) 
                    type_long = 1;
                if ( value[i] == 'L' ) 
                    type_long = 1;                  
                if( type_long != 0 && type_unsigned != 0){
                  returnT.typeIndex = PrimitiveTypes::U_LONG_T;
                  return returnT;
                }                
                i++;
            }
            
            if ( type_unsigned != 0 ) {
                returnT.typeIndex = PrimitiveTypes::U_INT_T;
                return returnT;
            }
            if ( type_long != 0 ) {
                returnT.typeIndex = PrimitiveTypes::LONG_T;
                return returnT;
            }
            
            returnT.typeIndex = PrimitiveTypes::INT_T;
            return returnT;
            
        } else if ( name == "CONSTANT EXP" ) {
            
            int i=0;
            while(i<len){
                if ( value[i] == 'F' || value[i] == 'f' ) {
                    returnT.typeIndex = PrimitiveTypes::FLOAT_T;
                    return returnT;
                }
            }
            returnT.typeIndex = PrimitiveTypes::LONG_T;
            return returnT;
        } else if ( name == "CONSTANT FLOAT" ) {
            
            int i=0;
            while(i<len){                
                if ( value[i] == 'F' || value[i] == 'f' ) {
                    returnT.typeIndex = PrimitiveTypes::FLOAT_T;
                    return returnT;
                }
                i++;
            }         
            returnT.typeIndex = PrimitiveTypes::DOUBLE_T;
            return returnT;
        }  else {
            return returnT;
        }
    }
};

class StringLiteral : public Terminal {
  public:
    StringLiteral( const char *name );
};

Expression *create_postfix_expr_arr( Expression *p_exp, Expression *exp );
Expression *create_postfix_expr_ido( string opr, Expression *p_exp );
Expression *create_postfix_expr_struct( string str, Expression *exp,Identifier *id );
Expression *create_toplevel_expression( Expression *exp1, Expression *exp2 );

#endif

