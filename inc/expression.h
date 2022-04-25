#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#include <deque>
#include <map>
#include <string>

#include "ast.h"
#include "symtab.h"
#include "3ac.h"

using namespace std;

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
  int num_operands;
  Address  * res;
  vector<GoTo*> truelist;
  vector<GoTo*> falselist;
  Expression() : Non_Terminal( "" ), res ( nullptr ){};
};

class ArgumentExprList : public Expression {
  public:
	vector <Expression * > args;

    ArgumentExprList() {};
};

ArgumentExprList *create_argument_expr_assignement( Expression *ase );
ArgumentExprList *create_argument_expr_list( ArgumentExprList *ae_list, Expression *ase );

class PrimaryExpression : public Expression {

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

Expression *create_primary_identifier( Identifier *a );
Expression *create_primary_constant( Constant *a );
Expression *create_primary_stringliteral( StringLiteral *a );
Expression *create_primary_expression( Expression *a );

class CastExpression : public Expression {
  public:
    Expression *op1;

    int typeCast;

    CastExpression() {
        op1 = nullptr;
        typeCast = -1;
    };
};
Expression *create_cast_expression_typename(TypeName *tn, Expression *ce );

class UnaryExpression : public Expression {
  public:
    Expression *op1;
    string op;

    UnaryExpression() {
        op1 = nullptr;
        op = "";
    }
};

Expression *create_unary_expression( Terminal * op, Expression *ue );
Expression *create_unary_expression( Terminal *, TypeName *t_name );
Expression *create_unary_expression_cast( Node *n_op, Expression *ce );


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
Expression *create_additive_expression( string op, Expression *ade, Expression *me );

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
Expression *create_multiplicative_expression( string op, Expression *me, Expression *ce );

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
Expression *create_relational_expression( string op, Expression *re, Expression *se );

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
Expression *create_equality_expression( string op, Expression *eq, Expression *re );

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

Expression *create_shift_expression( string op, Expression *se, Expression *ade );

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
Expression *create_and_expression( string op, Expression *an, Expression *eq );

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
Expression *create_inclusive_or_expression( string op, Expression *ie, Expression *ex );

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
Expression *create_exclusive_or_expression( string op, Expression *ex, Expression *an );


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
Expression *create_logical_or_expression( string op, Expression *lo, Expression *la );

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
Expression *create_logical_and_expression( string op, Expression *la, Expression *ie );

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

Expression *create_assignment_expression( Expression *ue, Node *n_op, Expression*ase ); 

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
Expression *create_conditional_expression( string op, Expression *lo, Expression *te, Expression *coe );

class ConstantExpression : public Expression {
  public:
    Expression *op1;
};

class TopLevelExpression : public Expression {
  public:
    Expression *op1;
    Expression *op2;

    TopLevelExpression() {
        op1 = nullptr;
        op2 = nullptr;
    };
};
Expression *create_toplevel_expression( Expression *te, Expression *ase );


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

union data{
    unsigned long ul;
    unsigned int ui;
    unsigned char uc;
    double d;
    long l;
    float f;
    char c;
    int i;
};
class Constant : public Terminal {
  public:
    Constant( const char *name ,const char* value, unsigned int line_no, unsigned int col_no);
    union data val;
    Type ConstantType;

    Type getConstantType() {
        return ConstantType;
    }
    void negate();
  
};

Constant* create_constant( const char *name ,const char* value, unsigned int line_no, unsigned int col_no);

class StringLiteral : public Terminal {
  public:
    StringLiteral( const char *name );
};

Expression *create_postfix_expr_voidfun( Identifier *fi );
Expression *create_postfix_expr_ido( Terminal * op, Expression *pe );
Expression *create_postfix_expr_arr( Expression *pe, Expression *exp );
Expression *create_postfix_expr_fun( Identifier *fi, ArgumentExprList *ae );
Expression *create_postfix_expr_struct( string access_op, Expression *pe, Identifier *id );
#endif
