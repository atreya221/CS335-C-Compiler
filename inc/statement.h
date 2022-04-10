#ifndef __STATEMENT_H__
#define __STATEMENT_H__

#include <deque>
#include <string>
#include <map>

#include "3ac.h"
#include "expression.h"
#include "symtab.h"
#include "ast.h"

using namespace std;


class Statement : public Non_Terminal {
    public:
        vector<GoTo*> break_vector;
        vector<GoTo*> continue_vector;
        vector<GoTo*> case_vector;
        vector<GoTo*> next_vector;
        vector<GoTo*> return_vector;
    Statement() : Non_Terminal("") {};   
    ~Statement() {
        next_vector.clear();
        break_vector.clear();
        continue_vector.clear();
        case_vector.clear();
    }
};

class TopLevelStatement : public Statement {
    public:
        Statement *statement1;

    TopLevelStatement() {};
    
};

class StatementList : public Statement{
    public :
};

Statement * create_statement_list( Statement * statement1 );
Statement * add_to_statement_list( Statement * statement1, Statement * statement2 );

class ExpressionStatement : public Statement{
    public:
        Expression *expression1;
        ExpressionStatement(){
            expression1 = nullptr;
        }

};

Statement* create_expression_statement(Expression* expression1);

class SelectionStatement : public Statement{
    public :
};
Statement *create_selection_statement_if( Expression *expression1, GoTo * _false, Label * label1, Statement *statement1, GoTo * _goto, Label * label2, Statement *statement2 );

void create_switch( Expression * expression );
Statement *create_selection_statement_switch(Expression *expression1, GoTo * _test, Statement* statement1, GoTo * _goto);

class IterationStatement : public Statement {
    public:
    IterationStatement(){}
};


Statement* create_iteration_statement_for( Expression * expression1, Label * label1,  Expression * expression2, Label * label2, Statement* statement1 );
Statement* create_iteration_statement_for( Expression * expression1, Label * label1, Expression * expression2, GoTo * _goto1, Label * label2, Expression * expression3, GoTo * _goto2 , Label * label3, Statement* statement1 );
Statement* create_iteration_statement_do_while(Label *label1,Statement *statement1,  Label *label2, Expression *expression1, GoTo * _false );
Statement* create_iteration_statement_while(Label * label1, Expression *expression1, GoTo * _false, Label * label2, Statement *statement1 );

class JumpStatement : public Statement {
    public:
        Identifier *id;
        Expression *expression1;
        string string_name; 
        JumpStatement() {
            id = nullptr;
            expression1 = nullptr;
            string_name = "";
        };
};

Statement* create_jump_statement( int type_ );
Statement* create_jump_statement_exp(Expression* expression);
Statement* create_jump_statement_go_to(Identifier* id);

class LabeledStatement: public Statement{
    public:
        Identifier *id;
        string string_name;
        Expression *expression;
        Statement *statement1;

        LabeledStatement() {
            id = nullptr;
            string_name="";
            expression = nullptr;
            statement1 = nullptr;
        };

};

Statement* create_labeled_statement_def(Label *label1, Statement* statement1);
Statement* create_labeled_statement_iden(Identifier *id, Label * label1, Statement* statement1);
Statement* create_labeled_statement_case(Constant *constant, Label* label1, Statement* statement1);

extern Type* switch_type;
extern Label * switch_temp;
extern map<string,Label *> label_iden;
extern map<string,vector<GoTo *> & > goto_iden;
extern map<string, Label *> switch_label;


class CompoundStatement : public Statement{
    public:
        StatementList *statement_list1;
        DeclarationList *declaration_list1;
        string string_name;
        CompoundStatement() {
            string_name = "";
            statement_list1=nullptr;
            declaration_list1=nullptr;
        };
};

Statement* create_compound_statement_1(string string_name,StatementList* statement_list1);
Statement* create_compound_statement_2(string string_name,DeclarationList* declaration_list1);
Statement* create_compound_statement_3(string string_name,DeclarationList* declaration_list1, StatementList* statement_list1);

void backpatch_fn( Statement * statement ) ;
#endif
