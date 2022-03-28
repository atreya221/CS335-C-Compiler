#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#include "ast.h"
#include <deque>
#include <map>
#include <string>
#include "symtab.h"

using namespace std;

// All class declarations
class StructDefinition;
class AssignmentExpression;
class ParameterTypeList;
class CastExpression;
class Constant;
class StringLiteral;
class TopLevelExpression;

class Expression : public Non_Terminal {
  public:
    Type type;
    /* Change this late */
    int num_opearands;
    // Expression( Types * type, int num_op );

    Expression() : Non_Terminal( "" ){};
};

// Expression::Expression(){}

//-------------------------------------------------
class PrimaryExpression : public Expression {
    /**
isTerminal = 0 when ( Expression )
isTerminal = 1 when IDENTIFIER
isTerminal = 2 when CONSTANT
isTerminal = 3 when STRING_LITERAL
*/
  public:
    int isTerminal;
    Identifier *Ival;
    StringLiteral *Sval;
    Constant *Cval;
    Expression *op1;

    PrimaryExpression() {
        isTerminal = 0;
        Ival = nullptr;
        Sval = nullptr;
        Cval = nullptr;
        op1 = nullptr;
    }
};

// Grammar warppers for PrimaryExpression
Expression *create_primary_identifier( Identifier *id );
Expression *create_primary_expression( Expression *exp );

//-------------------------------------------------

class ArgumentExprList : public Expression {
  public:
    Expression *op1; // This will be null in case of root object
    Expression *op2;

    ArgumentExprList() {
        op1 = nullptr;
        op2 = nullptr;
    };
};


//-------------------------------------------------
class UnaryExpression : public Expression {
  public:
    Expression *op1;
    string op;

    UnaryExpression() {
        op1 = nullptr;
        op = "";
    }
};

// Grammar warppers for UnaryExpression
Expression *
create_unary_expression_ue( string unary_op,
                            Expression *unary_exp ); // INC_OP, DEC_OP, SIZEOF
Expression *create_unary_expression_cast( Node *node_op, Expression *exp );

//-------------------------------------------------
class CastExpression : public Expression {
  public:
    Expression *op1;
    /**
      Index of Type casted to in GlobalTypes
      -1 if there is no casting
    */
    int typeCast;

    CastExpression() {
        op1 = nullptr;
        typeCast = -1;
    };
};

//-------------------------------------------------
class MultiplicativeExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;
    string op;

    MultiplicativeExpression() {
        op1 = nullptr;
        op2 = nullptr;
        op = "";
    };
};
Expression *create_multiplicative_expression( string opr, Expression *exp1,
                                              Expression *exp2 );

//-------------------------------------------------
class AdditiveExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;
    string op;

    AdditiveExpression() {
        op1 = nullptr;
        op2 = nullptr;
        op = "";
    };
};
Expression *create_additive_expression( string opr, Expression *exp1,
                                        Expression *exp2 );

//-------------------------------------------------
class ShiftExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;
    string op;

    ShiftExpression() {
        op1 = nullptr;
        op2 = nullptr;
        op = "";
    };
};

Expression *create_shift_expression( string opr, Expression *exp1,
                                     Expression *exp2 );

//-------------------------------------------------
class RelationalExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;
    string op;

    RelationalExpression() {
        op1 = nullptr;
        op2 = nullptr;
        op = "";
    };
};
Expression *create_relational_expression( string opr, Expression *exp1,
                                          Expression *exp2 );

//-------------------------------------------------
class EqualityExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;
    string op;

    EqualityExpression() {
        op1 = nullptr;
        op2 = nullptr;
        op = "";
    };
};
Expression *create_equality_expression( string opr, Expression *exp1,
                                        Expression *exp2 );

//--------------------------------------------------
class AndExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;
    string op;

    AndExpression() {
        op1 = nullptr;
        op2 = nullptr;
        op = "";
    };
};
Expression *create_and_expression( string opr, Expression *exp1,
                                   Expression *exp2 );

//-------------------------------------------------
class ExclusiveorExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;
    string op;

    ExclusiveorExpression() {
        op1 = nullptr;
        op2 = nullptr;
        op = "";
    };
};
Expression *create_exclusive_or_expression( string opr, Expression *exp1,
                                            Expression *exp2 );

//-------------------------------------------------
class InclusiveorExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;
    string op;

    InclusiveorExpression() {
        op1 = nullptr;
        op2 = nullptr;
        op = "";
    }
};
Expression *create_inclusive_or_expression( string opr, Expression *exp1, Expression *exp2 );

//-------------------------------------------------
class Logical_andExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;
    string op;

    Logical_andExpression() {
        op1 = nullptr;
        op2 = nullptr;
        op = "";
    };
};
Expression *create_logical_and_expression( string opr, Expression *exp1,
                                           Expression *exp2 );

//-------------------------------------------------
class Logical_orExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;
    string op;

    Logical_orExpression() {
        op1 = nullptr;
        op2 = nullptr;
        op = "";
    };
};
Expression *create_logical_or_expression( string opr, Expression *exp1,
                                          Expression *exp2 );

//-------------------------------------------------
class ConditionalExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;
    Expression *op3;
    string op;

    ConditionalExpression() {
        op1 = nullptr;
        op2 = nullptr;
        op3 = nullptr;
        op = "";
    };
};
Expression *create_conditional_expression( string opr, Expression *exp1,
                                           Expression *exp2, Expression *exp3 );

//-------------------------------------------------
class AssignmentExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;
    string op;

    AssignmentExpression() {
        op1 = nullptr;
        op2 = nullptr;
        op = "";
    }
};

Expression *create_assignment_expression(
    Expression *exp1, Node *node_opr,
    Expression
        *exp2 ); // can change string to node* later for assignment operator

//-------------------------------------------------
class TopLevelExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;

    TopLevelExpression() {
        op1 = nullptr;
        op2 = nullptr;
    };
};

//-------------------------------------------------
class ConstantExpression : public Expression {
  public:
    Expression *op1;
};

//-------------------------------------------------
class PostfixExpression : public Expression {
  public:
    PostfixExpression *pe;
    Expression *exp;
    Identifier *id;
    ArgumentExprList *ae_list;
    string op;

    PostfixExpression() {
        pe = nullptr;
        exp = nullptr;
        id = nullptr;
        ae_list = nullptr;
        op = "";
    };
};

class Constant : public Terminal {
  public:
    Constant( const char *name );

    Type getConstantType() {
        Type retT( 2, 0, false );
        int length = value.length();
        if ( name == "CONSTANT HEX" || name == "CONSTANT INT" ) {
          
            int islong = 0, isunsigned = 0;
            for ( int i = 0; i < length; i++ ) {
                if ( value[i] == 'l' || value[i] == 'L' )
                    islong = 1;
                if ( value[i] == 'u' || value[i] == 'U' )
                    isunsigned = 1;
                if ( islong && isunsigned ){
                    retT.typeIndex = PrimitiveTypes::U_LONG_T;
                    return retT;
                }
            }
            if ( islong ) {
                retT.typeIndex = PrimitiveTypes::LONG_T;
                return retT;
            }
            if ( isunsigned ) {
                retT.typeIndex = PrimitiveTypes::U_INT_T;
                return retT;
            }
            retT.typeIndex = PrimitiveTypes::INT_T;
            return retT;
            // loop over value to get unsigned etc and return typeIndex
        } else if ( name == "CONSTANT FLOAT" ) {
            for ( int i = 0; i < length; i++ ) {
                
                if ( value[i] == 'f' || value[i] == 'F') {
                    retT.typeIndex = PrimitiveTypes::FLOAT_T;
                    return retT;
                }
            }
            
            retT.typeIndex = PrimitiveTypes::DOUBLE_T;

            return retT;
            // loop over value to get float
        } else if ( name == "CONSANT EXP" ) {
            // loop over value to get if long or double
            for ( int i = 0; i < length; i++ ) {
                if ( value[i] == 'f' || value[i] == 'F' ) {
                    retT.typeIndex = PrimitiveTypes::FLOAT_T;
                    return retT;
                }
            }
            retT.typeIndex = PrimitiveTypes::LONG_T;
            return retT;
        } else {
            return retT;
        }
    }
};

class StringLiteral : public Terminal {
  public:
    StringLiteral( const char *name );
};

Expression *create_postfix_expr_arr( Expression *p_exp, Expression *exp );
Expression *create_postfix_expr_struct( string str, Expression *exp,
                                        Identifier *id );
Expression *create_postfix_expr_ido( string opr, Expression *p_exp );
Expression *create_toplevel_expression( Expression *exp1, Expression *exp2 );
#endif

