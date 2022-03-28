// symtab.cpp

#include "ast.h"
#include "symtab.h"
#include "expression.h"
#include "y.tab.h"

#include <assert.h>
#include <iostream>
#include <list>
#include <vector>
#include <utility>
#include <iterator>
#include <algorithm>
#include <sstream>

//declaring symbol tables over here
LocalSymbolTable local_symbol_table;
GlobalSymbolTable global_symbol_table;

std::vector<Types> defined_types;
unsigned int anon_count = 0;
extern int line_no;

std::string primitive_type_name( PrimitiveTypes type ) {
    std::string ss;
    switch ( type ) {
    case U_CHAR_T:
        ss = "unsigned char";
        break;
    case CHAR_T:
        ss = "char";
        break;
    case U_SHORT_T:
        ss = "unsigned short";
        break;
    case SHORT_T:
        ss = "short";
        break;
    case U_INT_T:
        ss = "unsigned int";
        break;
    case INT_T:
        ss = "int";
        break;
    case U_LONG_T:
        ss = "unsigned long";
        break;
    case LONG_T:
        ss = "long";
        break;
    case FLOAT_T:
        ss = "float";
        break;
    case DOUBLE_T:
        ss = "double";
        break;
    case LONG_DOUBLE_T:
        ss = "long double";
        break;
    case VOID_T:
        ss = "void";
        break;
    default:
        std::cerr << "Invalid Type" << type << "\n";
        assert( 0 );
    }
    return ss;
}

size_t primitive_type_size( PrimitiveTypes type ) {
    size_t size = 0;
    switch ( type ) {
    case U_CHAR_T:
        size = 1;
        break;
    case CHAR_T:
        size = 1;
        break;
    case U_SHORT_T:
        size = 2;
        break;
    case SHORT_T:
        size = 2;
        break;
    case U_INT_T:
        size = 4;
        break;
    case INT_T:
        size = 4;
        break;
    case U_LONG_T:
        size = 8;
        break;
    case LONG_T:
        size = 8;
        break;
    case FLOAT_T:
        size = 4;
        break;
    case DOUBLE_T:
        size = 8;
        break;
    case LONG_DOUBLE_T:
        size = 16;
        break;
    case VOID_T:
        size = 0;
        break;
    default:
        std::cerr << "Invalid Type";
        assert( 0 );
    }
    return size;
}

void instantiate_primitive_types() {
    int i = 0;
    while(i <= VOID_T){
        Types types;
        types.index = i;
        types.name = primitive_type_name((PrimitiveTypes) i);
        types.size = primitive_type_size((PrimitiveTypes) i);
        types.is_primitive = true;
        types.is_struct = false;
        types.is_union = false;
        types.struct_definition = NULL;
        defined_types.push_back(types);
        i++;
    }
}

//#####################################################################
// TYPE
//#####################################################################

Type::Type() {
    typeIndex = -1;
    ptr_level = -1;
    is_const = false;
}

Type::Type( int idx, int p_lvl, bool is_con ) {
    typeIndex = idx;
    ptr_level = p_lvl;
    is_const = is_con;
}
bool Type::isPrimitive() {
    if ( typeIndex >= 0 && typeIndex <= 12 ) {
        return true;
    } 
    return false;
}

std::string Type::get_name() {
    std::string ss;
    ss = defined_types[typeIndex].name;
    int i = 0;
    while(i < ptr_level){
        ss += "*";
        i++;
    }

    return ss;
}

int Type::get_size() {
    if ( ptr_level > 0 ) {
        //if is a pointer then the size is 8 bytes for that pointer
        return 8;
    }
    return defined_types[typeIndex].size;
}

bool Type::isInt() {
    if ( typeIndex >= 0 && typeIndex <= 7 && ptr_level == 0)
        return true;
    return false;
}

bool Type::isFloat() {
    if ( typeIndex >= 8 && typeIndex <= 10 && ptr_level == 0 )
        return true;
    
    return false;
}
bool Type::isIntorFloat() {
    if ( typeIndex <= 10 && ptr_level == 0 )
        return true;
    return false;
}

bool Type::isUnsigned() {
    if ( typeIndex == 0 || typeIndex == 2 || typeIndex == 4 ||
         typeIndex == 6 ) {
        return true;
    }
    return false;
}

bool Type::isPointer() {
    return !(ptr_level == 0);
}

void Type::make_signed() {
    if ( typeIndex == 0 || typeIndex == 2 || typeIndex == 4 ||
         typeIndex == 6 )
        typeIndex += 1;
}

void Type::make_unsigned() {
    if ( typeIndex == 1 || typeIndex == 3 || typeIndex == 5 ||
         typeIndex == 7 )
        typeIndex -= 1;
}

bool operator==( Type &obj1, Type &obj2 ) {
    return ( obj1.typeIndex == obj2.typeIndex &&
             obj1.ptr_level == obj2.ptr_level );
}

//############################################
//TYPES
//############################################

int add_to_defined_types( Types *type ) {
    auto it = defined_types.begin();
    while(it != defined_types.end()){
        if(it->name == type->name)
            return it->index;
        it++;
    }


    type->index = defined_types.size();
    defined_types.push_back( *type );
    return defined_types.size() - 1;
}

int get_type_index( std::string name ) {
    auto it = defined_types.begin();
    while(it != defined_types.end()){
        if(it->name == name){
            return it->index;
        }
        it++;
    }
    return -1;
}

void add_struct_defintion_to_type( int index,
                                   StructDefinition *struct_definition ) {
    defined_types[index].struct_definition = struct_definition;
    defined_types[index].size = struct_definition->get_size();
}

//######################################
//STRUCT DEFINITION
//######################################

StructDefinition::StructDefinition(){};

size_t StructDefinition::get_size() {
    size_t size = 0;
    auto it = members.begin();
    while(it != members.end()){
        size += it->second.get_size();
        it++;
    }
    
    return size;
}

StructDefinition *create_struct_definition( int un_or_st,
                                            StructDeclarationList *sdl ) {
    StructDefinition *sd = new StructDefinition();
    sd->un_or_st = un_or_st;
    
    std::cout << "struct {\n";
    auto it = sdl->struct_declaration_list.begin();
    while(it != sdl->struct_declaration_list.end()){
        int type_index = (*it)->sq_list->type_index;
        bool is_const = (*it)->sq_list->is_const;

        std::vector<Declarator *> dl =
            (*it)->declarator_list->declarator_list;
        auto it1 = dl.begin();
        while(it1 != dl.end()){
            int pointer_level = (*it1)->get_pointer_level();
            Type type(type_index, pointer_level, is_const);

            sd->members.insert({(*it1)->id->value, type});
            std::cout << "  " << ( *it1 )->id->value << " " << type.get_name() << "\n";
            it1++;
        }
            
        it++;
    }
    std::cout << "}\n";
    return sd;
}

Type *StructDefinition::get_member( Identifier *id ) {
    auto it = members.find( id->value );
    if(it != members.end()){
        return &(it->second);
    }
    return nullptr;
}

//###############################################
//POINTER
//###############################################
void is_Valid( TypeQualifierList *ts ) {

    unsigned int i = 0;
    while(i < ts->type_qualifier_list.size()){
        if ( ts->type_qualifier_list[i] == CONST ||
             ts->type_qualifier_list[i] == VOLATILE ) {
        } else {
            std::cout << "Error in pointer type qualfier pointer ";
            std::cerr << "on line " << line_no << "\n";
            break;
        }
        i++;
    }
}

Pointer::Pointer() : Non_Terminal( "pointer" ){};

Pointer::Pointer( TypeQualifierList *type_list, Pointer *pointer_ )
    : Non_Terminal( "pointer" ), type_qualifier_list(type_list),
      pointer(pointer_){};

Pointer *create_pointer() {
    Pointer *p = new Pointer();
    return p;
}

Pointer *create_pointer( TypeQualifierList *type_list, Pointer *pointer ) {
    Pointer *p = new Pointer( type_list, pointer );
    p->add_children( type_list, pointer );
    //to verify the type checking for pointer
    // is_Valid( type_list );
    return p;
}

//##############################################################################
//########################### TYPE QUALIFIER LIST
//##############################
//##############################################################################

TypeQualifierList ::TypeQualifierList()
    : Non_Terminal( "type_qualifier_list" ){};

TypeQualifierList ::~TypeQualifierList() { type_qualifier_list.clear(); };

void TypeQualifierList ::append_to_list( TYPE_QUALIFIER type ) {
    type_qualifier_list.push_back( type );
}


//##############################################################################
//############################# DECLARATION
//####################################
//##############################################################################

Declaration *new_declaration( DeclarationSpecifiers *declaration_specifiers, DeclaratorList *init_declarator_list ) {
    Declaration *d =
        new Declaration( declaration_specifiers, init_declarator_list );
    d->add_children( declaration_specifiers, init_declarator_list );
    declaration_specifiers->create_type();
    if(declaration_specifiers->type_specifier[0]->type==VOID)
    {
        std::cerr << "Error in strorage class declarator on line " << line_no << "\n";
        exit( 0 );
    }
    return d;
}

void Declaration::add_to_symbol_table( LocalSymbolTable &sym_tab ) {
    if ( init_declarator_list == nullptr ) {
        return;
    }
    std::vector<Declarator *> &dec = init_declarator_list->declarator_list;
    int type_index = declaration_specifiers->type_index;
    bool is_const = declaration_specifiers->is_const;
    assert( type_index != -1 );

    auto it = dec.begin();
    while(it != dec.end()){
        int pointer_level = (*it)->get_pointer_level();

        SymTabEntry *ste = new SymTabEntry( (*it)->id->value );
        ste->type = Type( type_index, pointer_level, is_const );
        sym_tab.add_to_table(ste);
        // CSV
        sym_tab.ss << "local, " << sym_tab.function_name << ", "
                   << (*it)->id->value << ", " << ste->type.get_name() << ", "
                   << sym_tab.current_level << "\n";
        it++;
    }

}


//#####################################
//DECLARATION
//#####################################

Declaration ::Declaration( DeclarationSpecifiers *declaration_specifiers_, DeclaratorList *init_declarator_list_ )
    : Non_Terminal( "declaration" ),
      declaration_specifiers( declaration_specifiers_ ),
      init_declarator_list( init_declarator_list_ ){};

void Declaration::add_to_symbol_table( GlobalSymbolTable &sym_tab ) {

    if ( init_declarator_list == nullptr )
        return;
    std::vector<Declarator *> &dec = init_declarator_list->declarator_list;

    int type_index = declaration_specifiers->type_index;
    bool is_const = declaration_specifiers->is_const;
    assert( type_index != -1 );

    auto it = dec.begin();
    while(it != dec.end()){
        int pointer_level = (*it)->get_pointer_level();

        SymTabEntry *ste = new SymTabEntry( (*it)->id->value );
        ste->type = Type(type_index, pointer_level, is_const);
        sym_tab.add_to_table(ste, true);
        sym_tab.ss << "global, "
                   << "-, " << ( *it )->id->value << ", " << ste->type.get_name()
                   << ", 0\n";
        it++;
    }

    write_to_symtab_file(sym_tab.ss.str());
    sym_tab.ss.clear();
    sym_tab.ss.str(std::string());
}

//##################################
//DECLARATION SPECIFIERS
//##################################

DeclarationSpecifiers ::DeclarationSpecifiers()
    : Non_Terminal( "declaration_specifiers" ), is_const( false ),
      type_index( -1 ){};

void DeclarationSpecifiers::create_type() {
    int err = 0;
    if (storage_class.size() == 0) {
    } else if ( storage_class.size() == 1 &&
                storage_class[0] == TYPEDEF ) {
        //TODO: to imlement typedef over here
    } else {
        err += 1;
    }

    std::vector<TYPE_SPECIFIER> ty;
    unsigned int i = 0;
    while(i < type_specifier.size()){
        ty.push_back(type_specifier[i]->type);
        i++;
    }

    std::sort( ty.begin(), ty.end() );

    is_const = false;
    type_index = -1;
    if (ty.size() == 3) {
        if ( ty[0] == UNSIGNED && ty[1] == LONG && ty[2] == LONG ) {
            type_index = U_LONG_T;
        } else {
            err += 2;
        }
    } else if ( ty.size() == 2 ) {
        if ( ty[0] == UNSIGNED && ty[1] == CHAR ) {
            type_index = U_CHAR_T;
        } else if ( ty[0] == SIGNED && ty[1] == CHAR ) {
            type_index = CHAR_T;
        } else if ( ty[0] == UNSIGNED && ty[1] == SHORT ) {
            type_index = U_SHORT_T;
        } else if ( ty[0] == SIGNED && ty[1] == SHORT ) {
            type_index = SHORT_T;
        } else if ( ty[0] == UNSIGNED && ty[1] == INT ) {
            type_index = U_INT_T;
        } else if ( ty[0] == SIGNED && ty[1] == INT ) {
            type_index = INT_T;
        } else if ( ty[0] == UNSIGNED && ty[1] == LONG ) {
            type_index = U_LONG_T;
        } else if ( ty[0] == SIGNED && ty[1] == LONG ) {
            type_index = LONG_T;
            //        } else if ( ty[0] == FLOAT && ty[1] ==
            //        DOUBLE )
            //        {
            //            type_index = DOUBLE_T;
        } else
            err += 2;
    } else if ( ty.size() == 1 ) {
        if ( ty[0] == FLOAT ) {
            type_index = FLOAT_T;
        } else if ( ty[0] == DOUBLE ) {
            type_index = DOUBLE_T;

        } else if ( ty[0] == INT ) {
            type_index = INT_T;

        } else if ( ty[0] == SHORT ) {
            type_index = SHORT_T;

        } else if ( ty[0] == CHAR ) {
            type_index = CHAR_T;

        }  else if ( ty[0] == VOID ) {
            type_index = VOID_T;
        } else if ( ty[0] == LONG ) {
            type_index = LONG_T;
        } else if ( ty[0] == ENUM ) {
            std::cerr << "ENUM not supported\n";
            exit( 0 );

        } else if ( ty[0] == STRUCT || ty[0] == UNION ) {
            type_index = type_specifier[0]->type_index;
        } else {
            err += 2;
        }
    }
    for ( unsigned int i = 0; i < type_qualifier.size(); i++ ) {

        if ( type_qualifier[i] == CONST ) {
            is_const = true;
        } else if ( type_qualifier[i] == VOLATILE ) {
        } else {
            err += 4;
            break;
        }
    }
    if ( err & 1 ) {
        std::cerr << "Error in strorage class declarator on line " << line_no << "\n";
        exit( 0 );
    }
    if ( err & 2 ) {
        std::cerr << "Error in type specifier declarator on line " << line_no << "\n";
        exit( 0 );
    }
    if ( err & 4 ) {
        std::cerr << "Error in type qualifier declarator on line " << line_no << "\n";
        exit( 0 );
    }
}

//#####################################
// DECLARATION LIST
//#####################################


DeclarationList::DeclarationList() : Non_Terminal( "declaration_list" ){};

//###############################
//DECLARATOR LIST
//###############################

DeclaratorList ::DeclaratorList() : Non_Terminal( "init_declarator_list" ){};

DeclaratorList *create_init_declarator_list( Declarator *d ) {
    DeclaratorList *dl = new DeclaratorList();
    dl->declarator_list.push_back( d );
    dl->add_child( d );
    return dl;
}

DeclaratorList *add_to_init_declarator_list( DeclaratorList *dl,
                                             Declarator *d ) {
    dl->declarator_list.push_back( d );
    dl->add_child( d );
    return dl;
}

//##################################
//DECLARATOR
//##################################

Declarator ::Declarator( Pointer *p, DirectDeclarator *dd ) : Non_Terminal( "declarator" ), pointer( p ), direct_declarator( dd ) {
    assert( dd != nullptr );
    assert( dd->id != nullptr );
    id = dd->id;
};

int Declarator::get_pointer_level() {
    Pointer *ptr = pointer;
    int count = 0;
    while ( ptr != NULL ) {
        count++;
        ptr = ptr->pointer;
    }
    return count;
}

Declarator *create_declarator( Pointer *pointer,
                               DirectDeclarator *direct_declarator ) {
    Declarator *d = new Declarator( pointer, direct_declarator );
    d->add_children( pointer, direct_declarator );
    return d;
}


//################################
//STRUCT VERIFY
//################################

void verify_struct_declarator( StructDeclarationList *st ) {
    int err;
    if ( st != NULL ) {
        // std::cout<<st->struct_declaration_list.size();
        unsigned int i = 0;
        while(i < st->struct_declaration_list.size()){
            std::vector<TypeSpecifier *> ts = st->struct_declaration_list[i]->sq_list->type_specifiers;
            std::vector<TYPE_QUALIFIER> tq =
                st->struct_declaration_list[i]->sq_list->type_qualifiers;
            
            err = 0;
            std::vector<TYPE_SPECIFIER> ty;
            unsigned int j = 0;
            while(j < ts.size()){
                ty.push_back(ts[j]->type);
                j++;
            }
            
            std::sort( ty.begin(), ty.end() );

            if ( ty.size() == 3 ) {
                if ( ( ty[0] == UNSIGNED || ty[0] == SIGNED ) && (ty[1] == SHORT || ty[1] == LONG ) && ty[2] == INT ) {
                } else
                    err += 2;
            } else if ( ty.size() == 2 ) {
                if ( ( ty[0] == UNSIGNED || ty[0] == SIGNED ) && ( ty[1] == SHORT || ty[1] == LONG || ty[1] == INT || ty[1] == CHAR ) ) {
                } else if ( ( ty[0] == SHORT || ty[0] == LONG ) && ty[1] == INT ) {
                } else if ( ty[0] == LONG && ty[1] == DOUBLE ) {
                } else
                    err += 2;
            } else if ( ty.size() == 1 ) {
                if ( ty[0] == SHORT || ty[0] == LONG ||
                     ty[0] == INT || ty[0] == CHAR ||
                     ty[0] == FLOAT || ty[0] == DOUBLE ||
                     ty[0] == STRUCT || ty[0] == UNION ||
                     ty[0] == ENUM ) {
                } else {
                    err += 2;
                }
            }
            j = 0;
            while(j < tq.size()){
                if ( tq[j] == CONST || tq[j] == VOLATILE ) {
                } else {
                    err += 4;
                    break;
                }
                j++;
            }
            if ( err & 2 ){
                std::cout << "Error in type specifier struct on line " << line_no << "\n";
                exit(0);
            }
            if ( err & 4 ){
                std::cout << "Error in type qualifier struct on line " << line_no << "\n";
                exit(0);
            }
            i++;
        }
        
    }
}
//##############################################################################
//############################ DIRECT DECLARATOR
//###############################
//##############################################################################

DirectDeclarator ::DirectDeclarator()
    : Non_Terminal( "direct_declarator" ), type( ID ), id( nullptr ),
      declarator( nullptr ), direct_declarator( nullptr ),
      const_expr( nullptr ), params( nullptr ){};

DirectDeclarator *create_dir_declarator_id( DIRECT_DECLARATOR_TYPE type, Identifier *id ) {
    assert( type == ID );
    DirectDeclarator *dd = new DirectDeclarator();
    dd->type = type;
    assert( id != nullptr );
    dd->id = id;
    dd->add_child( id );
    return dd;
}

DirectDeclarator *create_dir_declarator_dec( DIRECT_DECLARATOR_TYPE type, Declarator *declarator ) {
    assert( type == DECLARATOR );
    DirectDeclarator *dd = new DirectDeclarator();
    dd->type = type;
    assert( declarator != nullptr );
    dd->declarator = declarator;
    assert( declarator->direct_declarator != nullptr );
    assert( declarator->direct_declarator->id != nullptr );
    dd->id = declarator->direct_declarator->id;
    dd->add_child( declarator );
    return dd;
}

DirectDeclarator *create_dir_declarator_arr( DIRECT_DECLARATOR_TYPE type, DirectDeclarator *direct_declarator, Node *const_expr ){
    assert( type == ARRAY );
    DirectDeclarator *dd = new DirectDeclarator();
    dd->name = "direct_declarator_array";
    dd->type = type;
    assert( direct_declarator != nullptr );
    dd->direct_declarator = direct_declarator;
    dd->const_expr = const_expr;
    assert( direct_declarator->id != nullptr );
    dd->id = direct_declarator->id;
    dd->add_children( direct_declarator, const_expr );
    return dd;
}

DirectDeclarator *create_dir_declarator_fun( DIRECT_DECLARATOR_TYPE type, DirectDeclarator *direct_declarator, ParameterTypeList *params ) {
    assert( type == FUNCTION );
    DirectDeclarator *dd = new DirectDeclarator();
    dd->name = "direct_declarator_function";
    dd->type = type;
    assert( direct_declarator != nullptr );
    dd->direct_declarator = direct_declarator;
    dd->params = params;
    dd->id = direct_declarator->id;
    assert( direct_declarator->id != nullptr );
    dd->add_children( direct_declarator, params );
    return dd;
}

//##########################
//DIRECT ABSTRACT DECLARATOR
//##########################

DirectAbstractDeclarator::DirectAbstractDeclarator()
    : Non_Terminal( "direct_abstract_declarator" ), type( ABSTRACT ),
      abstract_declarator( nullptr ), const_expr( nullptr ),
      direct_abstract_declarator( nullptr ), parameter_type_list( nullptr ){};

DirectAbstractDeclarator *create_direct_abstract_declarator( DIRECT_ABSTRACT_DECLARATOR_TYPE typ ) {
    DirectAbstractDeclarator *dad = new DirectAbstractDeclarator();
    dad->type = typ;
    return dad;
}

DirectAbstractDeclarator *create_direct_abstract_declarator( DIRECT_ABSTRACT_DECLARATOR_TYPE typ, AbstractDeclarator *abs ) {
    assert( typ == ABSTRACT );
    DirectAbstractDeclarator *dad = new DirectAbstractDeclarator();
    dad->type = typ;
    dad->abstract_declarator = abs;
    dad->add_child( abs );
    return dad;
}

DirectAbstractDeclarator *create_direct_abstract_declarator( DIRECT_ABSTRACT_DECLARATOR_TYPE typ, DirectAbstractDeclarator *dabs ) {
    assert( typ == ROUND || typ == SQUARE );
    DirectAbstractDeclarator *dad = new DirectAbstractDeclarator();
    dad->type = typ;
    dad->direct_abstract_declarator = dabs;
    dad->add_child( dabs );
    return dad;
}

DirectAbstractDeclarator *create_direct_abstract_declarator( DIRECT_ABSTRACT_DECLARATOR_TYPE typ, DirectAbstractDeclarator *dabs, Node *ce ) {
    assert( typ == SQUARE );
    DirectAbstractDeclarator *dad = new DirectAbstractDeclarator();
    dad->type = typ;
    dad->direct_abstract_declarator = dabs;
    dad->const_expr = ce;
    dad->add_children( dabs, ce );
    return dad;
}
DirectAbstractDeclarator *create_direct_abstract_declarator( DIRECT_ABSTRACT_DECLARATOR_TYPE typ, DirectAbstractDeclarator *dabs, ParameterTypeList *param_list ) {
    assert( typ == ROUND );
    DirectAbstractDeclarator *dad = new DirectAbstractDeclarator();
    dad->type = typ;
    dad->direct_abstract_declarator = dabs;
    dad->parameter_type_list = param_list;
    dad->add_children( dabs, param_list );
    return dad;
}

//##############################
//ABSTRACT DECLARATOR
//##############################

AbstractDeclarator::AbstractDeclarator( Pointer *ptr, DirectAbstractDeclarator *dabs )
    : Non_Terminal( "abstract_declarator" ), pointer( ptr ), direct_abstract_declarator( dabs ){};

AbstractDeclarator *create_abstract_declarator( Pointer *ptr, DirectAbstractDeclarator *dabs ) {
    AbstractDeclarator *abs = new AbstractDeclarator( ptr, dabs );
    abs->add_children( ptr, dabs );
    return abs;
}
//############################
//PARAMETER DECLARATION
//############################
ParameterDeclaration::ParameterDeclaration()
    : Non_Terminal( "parameter_declaration" ){};

ParameterDeclaration *create_parameter_declaration( DeclarationSpecifiers *ds, Declarator *d, AbstractDeclarator *ad ) {
    ParameterDeclaration *pd = new ParameterDeclaration();
    pd->declaration_specifiers = ds;
    pd->declarator = d;
    pd->abstract_declarator = ad;
    pd->add_children( ds, d, ad );
    return pd;
}
//#############################
// PARAMETER TYPE LIST
//#############################

ParameterTypeList::ParameterTypeList()
    : Non_Terminal( "parameter_type_list" ){};


//##################################
// IDENTIFIER
//##################################

Identifier::Identifier( const char *name ) : Terminal( "IDENTIFIER", name ){};

//###############################
// FUNCTION DEFINITION
//###############################

FunctionDefinition::FunctionDefinition(
    DeclarationSpecifiers *declaration_specifiers_, Declarator *declarator_, Node *compound_statement_ )
    : Non_Terminal( "function_definiton" ),
      declaration_specifiers( declaration_specifiers_ ),
      declarator( declarator_ ), compound_statement( compound_statement_ ){};


//################################
// TYPE SPECIFIER
//################################

TypeSpecifier::TypeSpecifier( TYPE_SPECIFIER typ )
    : Non_Terminal( "type_specifier" ), type( typ ), id( nullptr ),
      struct_declaration_list( nullptr ), enumerator_list( nullptr ),
      type_index( -1 ){};

TypeSpecifier::TypeSpecifier( TYPE_SPECIFIER type_, Identifier *id_,
                              StructDeclarationList *struct_declaration_list_ )
    : Non_Terminal( "type_specifier" ), type( type_ ), id( id_ ),
      struct_declaration_list( struct_declaration_list_ ),
      enumerator_list( nullptr ), type_index( -1 ){};

TypeSpecifier::TypeSpecifier( TYPE_SPECIFIER type_, Identifier *id_,
                              EnumeratorList *enumerator_list_ )
    : Non_Terminal( "type_specifier" ), type( type_ ), id( id_ ),
      struct_declaration_list( nullptr ), enumerator_list( enumerator_list_ ),
      type_index( -1 ){};

TypeSpecifier *create_type_specifier( TYPE_SPECIFIER type ) {
    assert( type != UNION && type != STRUCT && type != ENUM );
    TypeSpecifier *ts = new TypeSpecifier( type );
    std::string ss;
    ss = "type_specifier : ";
    switch ( type ) {
    case INT:
        ss += "INT";
        break;
    case VOID:
        ss += "VOID";
        break;
    case CHAR:
        ss += "CHAR";
        break;
    case SHORT:
        ss += "SHORT";
        break;
    case LONG:
        ss += "LONG";
        break;
    case FLOAT:
        ss += "FLOAT";
        break;
    case DOUBLE:
        ss += "DOUBLE";
        break;
    case SIGNED:
        ss += "SIGNED";
        break;
    case UNSIGNED:
        ss += "UNSIGNED";
        break;
    case TYPE_NAME:
        ss += "TYPE_NAME";
        break;
    default:
        assert( 0 );
    }
    ts->name = ss;
    return ts;
}


TypeSpecifier *
create_type_specifier( TYPE_SPECIFIER type, Identifier *id, StructDeclarationList *struct_declaration_list ) {
    assert( type == UNION || type == STRUCT );
    TypeSpecifier *ts = new TypeSpecifier( type, id, struct_declaration_list );
    std::string ss;
    ss = "type_specifier : ";
    switch ( type ) {
    case UNION:
        ss += "UNION";
        break;
    case STRUCT:
        ss += "STRUCT";
        break;
    default:
        assert( 0 );
    }
    ts->name = ss;
    ts->add_children( id, struct_declaration_list );
    if ( struct_declaration_list != nullptr ) {
        verify_struct_declarator(struct_declaration_list);
        std::string struct_name;

        if ( id == nullptr ) {
            struct_name = "struct #anon_" + std::to_string( anon_count );
            anon_count++;
        } else {
            struct_name = "struct " + id->value;
        }

        Types *struct_type = new Types;
        struct_type->name = struct_name;
        struct_type->is_primitive = false;

        switch ( type ) {
        case UNION:
            struct_type->is_union = true;
            break;
        case STRUCT:
            struct_type->is_struct = true;
            break;
        default:
            assert( 0 );
        }
        ts->type_index = add_to_defined_types( struct_type );
        StructDefinition *struct_definition =
            create_struct_definition( type, struct_declaration_list );
        add_struct_defintion_to_type( ts->type_index, struct_definition );

    } else {
        ts->type_index = get_type_index( "struct " + id->value );
    }

    return ts;
}

TypeSpecifier *create_type_specifier( TYPE_SPECIFIER type, Identifier *id, EnumeratorList *enumerator_list ) {
    assert( type == ENUM );
    TypeSpecifier *ts = new TypeSpecifier( type, id, enumerator_list );
    std::string ss;
    ss = "type_specifier : ENUM";
    ts->name = ss;
    ts->add_children( id, enumerator_list );
    return ts;
}

//############################
// STRUCT DECLARATION LIST
//############################

StructDeclarationList::StructDeclarationList()
    : Non_Terminal( "struct_declaration_list" ){};

//##############################
// STRUCT DECLARATION
//##############################

StructDeclaration::StructDeclaration( SpecifierQualifierList *sq_list_, DeclaratorList *declarator_list_ )
    : Non_Terminal( "struct_declaration" ), sq_list( sq_list_ ),
      declarator_list( declarator_list_ ){};

//###########################
// SPECIFIER QUALIFIER LIST
//###########################

SpecifierQualifierList::SpecifierQualifierList()
    : Non_Terminal( "specifier_qualifier_list" ){};

void SpecifierQualifierList::create_type() {

    int err = 0;

    std::vector<TYPE_SPECIFIER> ty;
    unsigned int i = 0;
    while(i < type_specifiers.size()){
        ty.push_back( type_specifiers[i]->type );
        i++;
    }


    std::sort( ty.begin(), ty.end() );

    is_const = false;
    type_index = -1;
    if ( ty.size() == 3 ) {
        if ( ty[0] == UNSIGNED && ty[1] == LONG &&
             ty[2] == LONG ) {
            type_index = U_LONG_T;
        } else {
            err += 2;
        }
    } else if ( ty.size() == 2 ) {
        if ( ty[0] == UNSIGNED && ty[1] == CHAR ) {
            type_index = U_CHAR_T;
        } else if ( ty[0] == SIGNED && ty[1] == CHAR ) {
            type_index = CHAR_T;
        } else if ( ty[0] == UNSIGNED && ty[1] == SHORT ) {
            type_index = U_SHORT_T;
        } else if ( ty[0] == SIGNED && ty[1] == SHORT ) {
            type_index = SHORT_T;
        } else if ( ty[0] == UNSIGNED && ty[1] == INT ) {
            type_index = U_INT_T;
        } else if ( ty[0] == SIGNED && ty[1] == INT ) {
            type_index = INT_T;
        } else if ( ty[0] == UNSIGNED && ty[1] == LONG ) {
            type_index = U_LONG_T;
        } else if ( ty[0] == SIGNED && ty[1] == LONG ) {
            type_index = LONG_T;
            //        } else if ( ty[0] == FLOAT && ty[1] ==
            //        DOUBLE )
            //        {
            //            type_index = DOUBLE_T;
        } else
            err += 2;
    } else if ( ty.size() == 1 ) {
        if ( ty[0] == FLOAT ) {
            type_index = FLOAT_T;
        } else if ( ty[0] == DOUBLE ) {
            type_index = DOUBLE_T;

        } else if ( ty[0] == INT ) {
            type_index = INT_T;

        } else if ( ty[0] == SHORT ) {
            type_index = SHORT_T;

        } else if ( ty[0] == CHAR ) {
            type_index = CHAR_T;

        }  else if ( ty[0] == VOID ) {
            type_index = VOID_T;
        } else if ( ty[0] == LONG ) {
            type_index = LONG_T;
        } else if ( ty[0] == ENUM ) {
            std::cerr << "ENUM not supported\n";
            exit( 0 );

        } else if ( ty[0] == STRUCT || ty[0] == UNION ) {
            type_index = type_specifiers[0]->type_index;
        } else {
            err += 2;
        }
    }

    i = 0;
    while(i < type_qualifiers.size()){
        if ( type_qualifiers[i] == CONST ) {
            is_const = true;
        } else if ( type_qualifiers[i] == VOLATILE ) {
        } else {
            err += 4;
            break;
        }
        i++;
    }

    if ( err & 1 ) {
        std::cerr << "Error in strorage class declarator on line " << line_no << "\n";
        exit( 0 );
    }
    if ( err & 2 ) {
        std::cerr << "Error in type specifier declarator on line " << line_no << "\n";
        exit( 0 );
    }
    if ( err & 4 ) {
        std::cerr << "Error in type qualifier declarator on line " << line_no << "\n";
        exit( 0 );
    }
}


//##################################
// ENUMERATOR
//##################################

Enumerator::Enumerator( Identifier *id_, Node *const_expr )
    : Non_Terminal( "enumerator" ), id( id_ ), init_expr( const_expr ){};

Enumerator *create_enumerator( Identifier *id, Node *const_expr ) {
    Enumerator *e = new Enumerator( id, const_expr );
    e->add_children( id, const_expr );
    return e;
}

//###############################
// ENUMERATOR LIST
//###############################

EnumeratorList::EnumeratorList() : Non_Terminal( "enumerator_list" ){};

//##############################
// LOCAL SYMBOL TABLE
//##############################

SymbolTable::SymbolTable(){};

void SymbolTable::add_to_table( SymTabEntry * ) {}

SymTabEntry *SymbolTable::get_symbol_from_table( std::string name ) {
    return nullptr;
}

LocalSymbolTable::LocalSymbolTable() : current_level( 0 ){};

void LocalSymbolTable::increase_level() {
    for ( int i = 0; i < current_level; i++ ) {
        ; // std::cout << "  ";
    }
    // std::cout << "{\n";
    current_level++;
    write_to_symtab_file( ss.str() );
    ss.clear();
    ss.str( std::string() );
}

void LocalSymbolTable::clear_current_level() {
    auto it = sym_table.begin();
    while(it != sym_table.end()){
        if ( it->second.empty() ) {
            continue;
        }
        SymTabEntry *entry = ( it->second ).front();
        if ( !entry ) {
            continue;
        } else if ( entry->level == current_level ) {
            it->second.pop_front();
        }
        it++;
    }
    
    current_level--;
    // for ( int i = 0; i < current_level; i++ ) {
        // std::cout << "  ";
    // }
    // std::cout << "}\n";
    write_to_symtab_file( ss.str() );
    ss.clear();
    ss.str( std::string() );
}

void LocalSymbolTable::add_to_table( SymTabEntry *symbol ) {

    auto it = sym_table.find( symbol->name );
    if ( it == sym_table.end() ) {
        std::deque<SymTabEntry *> &q = *new std::deque<SymTabEntry *>;
        symbol->level = current_level;
        q.push_front( symbol );
        sym_table.insert( {symbol->name, q} );
    } else {
        std::deque<SymTabEntry *> &q = it->second;
        if ( q.front() && ( q.front() )->level == current_level ) {
            std::cout << "\nERROR: Redeclaration of symbol " << it->first
                      << " on line:" << line_no << " in the scope.\n";
            exit( 1 );
        } else {
            symbol->level = current_level;
            q.push_front( symbol );
        }
    }
}

SymTabEntry *LocalSymbolTable::get_symbol_from_table( std::string name ) {
    auto it = sym_table.find( name );
    if ( it == sym_table.end() ) {
        return nullptr;
    } else {
        if ( it->second.empty() ) {
            return nullptr;
        } else {
            return it->second.front();
        }
    }
}

void LocalSymbolTable::add_function(
    DeclarationSpecifiers *declaration_specifiers, Declarator *declarator ) {

    sym_table.clear();
    ss.clear();
    ss.str( std::string() );
    assert( declarator->direct_declarator->type == FUNCTION );
    function_name = declarator->id->value;
    // std::cout << "L: " << function_name << " " <<
    // declaration_specifiers->type_index;

    // Check whether the arguements are of the form ( )
    if ( declarator->direct_declarator->params == nullptr ) {
        // std::cout << "( )\n";
        return;
    }
    std::vector<ParameterDeclaration *> &param_list =
        declarator->direct_declarator->params->param_list;

    // Check whether the arguements are of the form ( void )
    if ( param_list.size() == 1 ) {
        auto it = param_list.begin();
        if ( ( *it )->declarator == nullptr &&
             ( *it )->abstract_declarator == nullptr ) {
            std::vector<TypeSpecifier *> &v =
                ( *it )->declaration_specifiers->type_specifier;
            if ( v.size() == 1 && ( *v.begin() )->type == VOID ) {
                // std::cout << "( void )\n";
                return;
            }
        }
    }
    // Parameters need to be added at level one, we avoid calling
    // increase_level and clear_from_level to avoid thier side effects

    current_level = 1;
    // std::cout << "( ";
    auto it = param_list.begin();
    while(it != param_list.end()){
        if ( ( *it )->declarator == nullptr ||
             ( *it )->declarator->id == nullptr ) {
            std::cout << "\nERROR: Arguement requires identifier on line:"
                      << line_no << "\n";
            exit( 1 );
        }

        ( *it )->declaration_specifiers->create_type();
        int type_index = ( *it )->declaration_specifiers->type_index;
        bool is_const = ( *it )->declaration_specifiers->is_const;
        int pointer_level = ( *it )->declarator->get_pointer_level();

        Type t( type_index, pointer_level, is_const );
        ss << "local, " << function_name << ", " << ( *it )->declarator->id->value
           << ", " << t.get_name() << ", "
           << "arg"
           << "\n";

        // std::cout << ( *it )->declarator->id->value << " " << type_index <<
        // ", ";
        SymTabEntry *symbol = new SymTabEntry( ( *it )->declarator->id->value );
        add_to_table( symbol );
        it++;
    }
    current_level = 0;
    // std::cout << " )\n";
    write_to_symtab_file( ss.str() );
    ss.clear();
    ss.str( std::string() );
}

//#################################
// GLOBAL SYMBOL TABLE
//#################################

void GlobalSymbolTable::add_symbol(
    DeclarationSpecifiers *declaration_specifiers, Declarator *declarator ) {

    declaration_specifiers->create_type();
    int type_index = declaration_specifiers->type_index;
    bool is_const = declaration_specifiers->is_const;
    int pointer_level = declarator->get_pointer_level();

    SymTabEntry *e = new SymTabEntry( declarator->id->value );
    e->type = Type( type_index, pointer_level, is_const );
    add_to_table( e, false );
    ss << "global, "
       << "-, " << declarator->id->value << ", fun: " << e->type.get_name()
       << ", 0\n";

    write_to_symtab_file( ss.str() );
    ss.clear();
    ss.str( std::string() );

    // std::cout << "G: " << declarator->id->value << "\n";
}

SymTabEntry *GlobalSymbolTable::get_symbol_from_table( std::string name ) {
    auto it = sym_table.find( name );
    if ( it == sym_table.end() ) {
        return nullptr;
    } else {
        return it->second;
    }
}

void GlobalSymbolTable::add_to_table( SymTabEntry *symbol, bool redef ) {

    auto it = sym_table.find( symbol->name );
    if ( it == sym_table.end() ) {
        sym_table.insert( {symbol->name, symbol} );
    } else if ( it->second->type == symbol->type && redef ) {

        // Can't insert two symbols with same name at the same level
        std::cout << "\nERROR: Redeclaration of symbol " << it->first
                  << " on line:" << line_no << " in the scope.\n";
        exit( 1 );
    }
}


void write_to_symtab_file( std::string s ) { symbol_file << s; }

//##############################
// SYMBOL TABLE ENTRY
//##############################

SymTabEntry::SymTabEntry( std::string name ) : name( name ){};

