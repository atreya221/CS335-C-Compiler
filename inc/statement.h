#ifndef __STATEMENT_H__
#define __STATEMENT_H__

#include <deque>
#include <map>
#include <string>

#include "ast.h"
#include "symtab.h"
#include "3ac.h"

using namespace std;


class Statement : public Non_Terminal {
    public:
        vector<GoTo*> nextlist; 
        vector<GoTo*> breaklist;
        vector<GoTo*> continuelist;
        vector<GoTo*> caselist;
        vector<GoTo*> returnlist;
    Statement() : Non_Terminal("") {};   
    ~Statement() {
        nextlist.clear();
        breaklist.clear();
        continuelist.clear();
        caselist.clear();
    }
};

class TopLevelStatement : public Statement {
    public:
        Statement *s1;

    TopLevelStatement() {};
    
};

class StatementList : public Statement{
    public :
};

Statement * create_statement_list( Statement * st1 );
Statement * add_to_statement_list( Statement * stl, Statement * st1 );

class ExpressionStatement : public Statement{
    public:
        Expression *e1;
        ExpressionStatement(){
            e1=nullptr;
        }

};

Statement* create_expression_statement(Expression* e1);

class SelectionStatement : public Statement{
    public :
};
Statement *create_selection_statement_if( Expression *ex, GoTo * _false, Label * l1, Statement *st1, GoTo * _goto, Label * l2, Statement *st2 );

void create_switch( Expression * ex );
Statement *create_selection_statement_switch(Expression *ex1, GoTo * _test, Statement* st1, GoTo * _goto);

class IterationStatement : public Statement {
    public:
    IterationStatement(){}
};

Statement* create_iteration_statement_while(Label * l1, Expression *e1, GoTo * _false, Label * l2, Statement *s1 );
Statement* create_iteration_statement_do_while(Label *l1,Statement *s1,  Label *l2,Expression *e1, GoTo * _false );
Statement* create_iteration_statement_for( Expression * ex1, Label * l1,  Expression * ex2, Label * l2, Statement*s1 );
Statement* create_iteration_statement_for( Expression * ex1, Label * l1, Expression * ex2, GoTo * _goto1, Label * l2, Expression * ex3, GoTo * _goto2 , Label * l3, Statement*s1 );

class JumpStatement : public Statement {
    public:
        Expression *st1;
        Identifier *id;
        string st; 
        JumpStatement() {
            st1=nullptr;
            id=nullptr;
            st="";
        };
};

Statement* create_jump_statement_go_to(Identifier* id);
Statement* create_jump_statement( int type_ );
Statement* create_jump_statement_exp(Expression* ex);

class LabeledStatement: public Statement{
    public:
        Statement *st1;
        Identifier *id;
        string st;
        Expression *ex;
        LabeledStatement() {
            st1=nullptr;
            ex=nullptr;
            st="";
            ex=nullptr;  
        };

};

Statement* create_labeled_statement_iden(Identifier *id,Label * l1, Statement* s1);
Statement* create_labeled_statement_case(Constant *con,Label* l,Statement* s1);
Statement* create_labeled_statement_def(Label *l,Statement* s1);

extern map<string,Label *> label_iden;
extern map<string,vector<GoTo *> & > goto_iden;
extern map<string, Label *> switch_label;
extern Type* typeSwitch;
extern Label * switch_temp;
class CompoundStatement : public Statement{
    public:
        StatementList *sl1;
        DeclarationList *dl1;
        string st;
        CompoundStatement() {
            st="";
            sl1=nullptr;
            dl1=nullptr;
        };
};

Statement* create_compound_statement_1(string st,StatementList* sl1);
Statement* create_compound_statement_2(string st,DeclarationList* dl1);
Statement* create_compound_statement_3(string st,DeclarationList* dl1,StatementList* sl1);

void backpatch_fn( Statement * s ) ;
#endif
