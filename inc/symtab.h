#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include "ast.h"
#include <deque>
#include <map>
#include <string>
#include <sstream>

using namespace std;

void yyerror(const char *);
extern int error_flag;

class Expression;
class PrimaryExpression;
class Identifier;

Expression *create_assignment_expression(Expression* exp1, Node *node_op, Expression* exp2);


enum PrimitiveTypes {
    ERROR_T = -1,
    U_CHAR_T = 0,
    CHAR_T = 1,
    U_SHORT_T = 2,
    SHORT_T = 3,
    U_INT_T = 4,
    INT_T = 5,
    U_LONG_T = 6,
    LONG_T = 7,
    FLOAT_T = 8,
    DOUBLE_T = 9,
    LONG_DOUBLE_T = 10,
    VOID_T = 11
};

extern int line_no;
extern int prev_line_no;
extern int col_no;
extern int prev_col_no;

extern stringstream text;
extern vector<string> code;
#define WORD_SIZE 4

// classes which are defined in expressions header file
class StructDefinition;
class AssignmentExpression;
class ParameterTypeList;
class CastExpression;
class Constant;
class StringLiteral;
class TopLevelExpression;

class Types;
class Type;

extern vector<Types> defined_types;

string primitive_type_name( PrimitiveTypes type );
size_t primitive_type_size( PrimitiveTypes type );
void instantiate_primitive_types();

class StructDeclarationList;
class StructDefinition {
  private:
    int recursive;
  public:
    int un_or_st;
    map<string, Type> members;
    map<string, size_t> offsets;

    StructDefinition();
    size_t get_size();
    size_t get_offset(Identifier *id);
    Type *get_member( Identifier *id );
};

StructDefinition *create_struct_definition( int un_or_st,
                                            StructDeclarationList *sdl );

class Types {
  public:
    int index;
    string name;
    size_t size;
    bool is_primitive;
    bool is_struct;
    bool is_union;
    StructDefinition *struct_definition;

    Types();
};

class Type {
  public:
    int typeIndex;

    bool is_pointer;
    int ptr_level;

    //For array
    bool is_arr;
    int arr_size;
    vector<int> arr_sizes;

    //For functions
    bool is_func;
    int no_args;
    vector<Type> arg_types;
    bool is_defined;

    bool is_const;

    Type();

    Type( int idx, int p_lvli, bool is_con );
    bool isPrimitive();
    string get_name();
    bool isVoid();
    bool isChar();
    bool isInt();
    bool isFloat();
    bool isIntorFloat();
    bool isUnsigned();
    bool isPointer();
    bool is_invalid();
    bool is_ea();

    void make_signed();
    void make_unsigned();
    
    size_t get_size();


    friend bool operator==(Type &type1, Type &type2);
    friend bool operator!=(Type &type1, Type &type2);
};

extern Type invalid_type;

extern vector<Types *> type_specifiers;

int add_to_defined_types( Types *type );

class ParameterTypeList;

// extern ofstream symbol_file;
extern stringstream symbol_stream;

void write_to_symtab_file( string s );

class SymTabEntry {
  public:
    string name;
    int level;
    Type type;
    int line_no;
    int col_no;
    size_t offset;
    int id;
    //We can add further properties to the symbol table entry
    SymTabEntry( string name_parameter, int line_no, int col_no);
};

class FuncEnt : public SymTabEntry {
  public:
    string args;
    FuncEnt( string name, string type, string args );
};

class DeclarationSpecifiers;
class Declarator;

class SymbolTable {

  public:
    SymbolTable();

    int symbol_id;
    stringstream ss;
    virtual SymTabEntry *get_symbol_from_table( string name );
    void delete_from_table( SymTabEntry *symbol );
    void print_table();
    virtual void add_to_table( SymTabEntry *symbol );
};

#define LOCAL_SYMBOL_MASK 0x10000000
#define GLOBAL_SYMBOL_MASK 0x20000000
#define FUN_ARG_MASK 0x40000000

class GlobalSymbolTable : public SymbolTable {
  public:
    size_t offset;
    map<string, SymTabEntry *> sym_table;
    void add_symbol( DeclarationSpecifiers *declaration_specifiers,
                     Declarator *declarator, int *error );
    void add_to_table( SymTabEntry *symbol, bool redef, Identifier *id );
    SymTabEntry *get_symbol_from_table( string name );
};

class LocalSymbolTable : public SymbolTable {
  public:
    map<string, deque<SymTabEntry *> &> sym_table;
    string function_name;
    int current_level;
    size_t offset;
    size_t reqd_size;
	Type return_type;
    void increase_level();
    void clear_current_level();
    void empty_table();
    LocalSymbolTable();
    void add_to_table( SymTabEntry *symbol, Identifier *id , bool arg1 );
    SymTabEntry *get_symbol_from_table( string name );
    void add_function( DeclarationSpecifiers *declaration_specifiers,
                       Declarator *declarator, int *err );
};

extern LocalSymbolTable local_symbol_table;
extern GlobalSymbolTable global_symbol_table;

typedef int TYPE_QUALIFIER;

class Identifier : public Terminal {
  public:
    Identifier( const char *name );
    Identifier( const char *name, int _line_no, int _col_no);
};

class TypeQualifierList : public Non_Terminal {
  public:
    vector<TYPE_QUALIFIER> type_qualifier_list;

    TypeQualifierList();

    void append_to_list( TYPE_QUALIFIER type );

    ~TypeQualifierList();
};

void is_Valid( TypeQualifierList *ts );
class Pointer : public Non_Terminal {
  public:
    TypeQualifierList *type_qualifier_list;
    Pointer *pointer;

    Pointer();
    Pointer( TypeQualifierList *type_qualifier_list, Pointer *pointer );
};

Pointer *create_pointer();
Pointer *create_pointer( TypeQualifierList *type_list, Pointer *pointer );

class DirectDeclarator;

class Declarator : public Non_Terminal {
  public:
    Identifier *id;
    Pointer *pointer;
    DirectDeclarator *direct_declarator;
    
    Expression *init_expr;
    Terminal *eq;
    int get_pointer_level();
    Declarator();
    Declarator( Pointer *p, DirectDeclarator *dd );
};

                 
Declarator *create_declarator( Pointer *pointer,
                               DirectDeclarator *direct_declarator );

typedef enum direct_declartor_enum {
    ID,
    DECLARATOR,
    ARRAY,
    FUNCTION,
    NUM_DIRECT_DECLARATORS

} DIRECT_DECLARATOR_TYPE;

class DirectDeclarator : public Non_Terminal {

  public:
    DIRECT_DECLARATOR_TYPE type;

    Identifier *id;
#if 0
    Declarator *declarator;
    DirectDeclarator *direct_declarator;


		Constant_Expression * const_expr;
#endif

    vector<int> arr_sizes;
    ParameterTypeList *params;
    DirectDeclarator();
};

DirectDeclarator *create_dir_declarator_id( DIRECT_DECLARATOR_TYPE type,
                                            Identifier *id );
DirectDeclarator *create_dir_declarator_dec( DIRECT_DECLARATOR_TYPE type,
                                             Declarator *declarator );
DirectDeclarator *
append_dir_declarator_arr( DIRECT_DECLARATOR_TYPE type,
                           DirectDeclarator *direct_declarator,
                           Constant *const );
DirectDeclarator *
append_dir_declarator_fun( DIRECT_DECLARATOR_TYPE type,
                           DirectDeclarator *direct_declarator,
                           ParameterTypeList *params );

class DeclaratorList : public Non_Terminal {
  public:
    vector<Declarator *> declarator_list;
    DeclaratorList();
};

DeclaratorList *create_init_declarator_list( Declarator *init_declarator );
DeclaratorList *add_to_init_declarator_list( DeclaratorList *init_declarator_list,Declarator *init_declarator );


typedef int STORAGE_CLASS;
class TypeSpecifier;
int get_index( Type t );
int set_index( DeclarationSpecifiers *ds );

class DeclarationSpecifiers : public Non_Terminal {
  public:
    vector<STORAGE_CLASS> storage_class;
    vector<TypeSpecifier *> type_specifier;
    vector<TYPE_QUALIFIER> type_qualifier;
    bool is_const;
    int type_index;

    void create_type(); // Type Checking

    DeclarationSpecifiers();
};

class Declaration : public Non_Terminal {
  public:
    DeclarationSpecifiers *declaration_specifiers;
    DeclaratorList *init_declarator_list;
    int type;
    Declaration( DeclarationSpecifiers *declaration_specifiers_,
                 DeclaratorList *init_declarator_list_ );
    void add_to_symbol_table( LocalSymbolTable &sym_tab );
    void add_to_symbol_table( GlobalSymbolTable &sym_tab );
    void dotify();
};

Declaration *new_declaration( DeclarationSpecifiers *declaraion_specifiers,
                              DeclaratorList *init_declarator_list );

class DeclarationList : public Non_Terminal {
  public:
    vector<Declaration *> declarations;
    DeclarationList();

    void create_symbol_table_level();
};

class FunctionDefinition : public Non_Terminal {
  public:
    DeclarationSpecifiers *declaration_specifiers;
    Declarator *declarator;
    Node *compound_statement;
    int error;

    FunctionDefinition( DeclarationSpecifiers *declaration_specifiers_,
                        Declarator *declarator_, Node *compound_statement_ );
};

int isValid(); // Type Checking;

class AbstractDeclarator;

typedef enum {
    ABSTRACT,
    SQUARE,
    ROUND
} DIRECT_ABSTRACT_DECLARATOR_TYPE;

class DirectAbstractDeclarator : public Non_Terminal {
  public:
    vector<int> arr_sizes;
    DirectAbstractDeclarator();
};

DirectAbstractDeclarator *
create_direct_abstract_declarator( Constant *_const);
DirectAbstractDeclarator *
append_direct_abstract_declarator( DirectAbstractDeclarator *dabs,
                                   Constant *_const );

#if 0
DirectAbstractDeclarator *
create_direct_abstract_declarator( DIRECT_ABSTRACT_DECLARATOR_TYPE typ,
                                   DirectAbstractDeclarator *abs );
DirectAbstractDeclarator *
create_direct_abstract_declarator( DIRECT_ABSTRACT_DECLARATOR_TYPE typ,
                                   DirectAbstractDeclarator *dabs, Node *ce );
DirectAbstractDeclarator *
create_direct_abstract_declarator( DIRECT_ABSTRACT_DECLARATOR_TYPE typ,
                                   DirectAbstractDeclarator *dabs,
                                   ParameterTypeList *param_list );
#endif
class AbstractDeclarator : public Non_Terminal {
  public:
    Pointer *pointer;
    DirectAbstractDeclarator *direct_abstract_declarator;
    AbstractDeclarator( Pointer *ptr, DirectAbstractDeclarator *dabs );
    int get_pointer_level();
};

AbstractDeclarator *
create_abstract_declarator( Pointer *pointer, DirectAbstractDeclarator *dabs );

class SpecifierQualifierList;

class TypeName : public Non_Terminal {
  public:
    SpecifierQualifierList *sq_list;
    AbstractDeclarator *abstract_declarator;
    Type type;
    TypeName();
};

TypeName *create_type_name( SpecifierQualifierList *, AbstractDeclarator * );

class ParameterDeclaration : public Non_Terminal {
  public:
    DeclarationSpecifiers *declaration_specifiers;
    Declarator *declarator;
    AbstractDeclarator *abstract_declarator;
    Type type;
    ParameterDeclaration();
    void create_type();
};

ParameterDeclaration *create_parameter_declaration( DeclarationSpecifiers *ds,
                                                    Declarator *d,
                                                    AbstractDeclarator *ad );

class ParameterTypeList : public Non_Terminal {
  public:
    vector<ParameterDeclaration *> param_list;
    bool has_ellipsis;
    ParameterTypeList();
};

class SpecifierQualifierList : public Non_Terminal {
  public:
    vector<TypeSpecifier *> type_specifiers;
    vector<TYPE_QUALIFIER> type_qualifiers;

    bool is_const;
    int type_index;

    void create_type(); // Type Checking

    SpecifierQualifierList();
};

class StructDeclaration : public Non_Terminal {
  public:
    SpecifierQualifierList *sq_list;
    DeclaratorList *declarator_list;

    StructDeclaration( SpecifierQualifierList *sq_list_,
                       DeclaratorList *declarator_list_ );
    void add_to_struct_definition( StructDefinition * );
};

class StructDeclarationList : public Non_Terminal {
  public:
    vector<StructDeclaration *> struct_declaration_list;
    StructDeclarationList();
};

int verify_struct_declarator( StructDeclarationList *st );
class Enumerator : public Non_Terminal {
  public:
    Identifier *id;
    Node *init_expr;
    Enumerator( Identifier *id_, Node *init_expr );
};

Enumerator *create_enumerator( Identifier *id, Node *const_expr );

class EnumeratorList : public Non_Terminal {
  public:
    vector<Enumerator *> enumerator_list;
    EnumeratorList();
};


typedef int TYPE_SPECIFIER;

class TypeSpecifier : public Terminal {
  public:
    TYPE_SPECIFIER type;
    Identifier *id;
    StructDeclarationList *struct_declaration_list;
    EnumeratorList *enumerator_list;
    int type_index;

    TypeSpecifier( TYPE_SPECIFIER typ, int line_no, int col_no );
    TypeSpecifier( TYPE_SPECIFIER type, Identifier *Id,
                   StructDeclarationList *struct_declaration_list );
    TypeSpecifier( TYPE_SPECIFIER type, Identifier *Id,
                   EnumeratorList *enumerator_list );
};

TypeSpecifier *create_type_specifier( TYPE_SPECIFIER type, int line_no, int col_no );

TypeSpecifier *create_struct_type( TYPE_SPECIFIER type, Identifier *id );

TypeSpecifier * add_struct_declaration( TypeSpecifier * ts,  StructDeclarationList *struct_declaration_list );

TypeSpecifier *create_type_specifier( TYPE_SPECIFIER type, Identifier *id, EnumeratorList *enumerator_list );


void error_message( string str, int ln_no, int col_no );
void error_message( string str, int ln_no );
void warning_message( string str, int ln_no, int col_no );
void warning_message( string str, int ln_no );

#endif
