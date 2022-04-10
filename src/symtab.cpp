// symtab.cpp

#include "ast.h"
#include "symtab.h"
#include "expression.h"
#include "statement.h"
#include "y.tab.h"

#include <assert.h>
#include <iostream>
#include <list>
#include <vector>
#include <utility>
#include <iterator>
#include <algorithm>
#include <sstream>

using namespace std;
//declaring symbol tables over here
LocalSymbolTable local_symbol_table;
GlobalSymbolTable global_symbol_table;

vector<Types> defined_types;
unsigned int anon_count = 0;
extern int line_no;

string primitive_type_name( PrimitiveTypes type ) {
    string ss;
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
        cerr << "Invalid Type" << type << "\n";
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
        cerr << "Invalid Type";
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
    is_pointer = false;
    is_arr = false;
    arr_size = 0;
    is_func = false;
    no_args = 0;
}

Type::Type( int idx, int p_lvl, bool is_con ) {
    typeIndex = idx;
    is_const = is_con;
    ptr_level = p_lvl;
    is_pointer = ptr_level > 0 ? true : false;
    
    is_arr = false;
    arr_size = 0;

    is_func = false;
    no_args = 0;
    is_defined = false;

}

bool Type::isPrimitive() {
    if ( typeIndex >= 0 && typeIndex <= VOID_T ) {
        return true;
    } 
    return false;
}

string Type::get_name() {
    string ss;
    ss = defined_types[typeIndex].name;

    if(is_arr){
        ss += " ";
        for(int i = 0; i < arr_size; i++){
            if(arr_sizes[i] != 0){
                ss += "[";
                ss += arr_sizes[i];
                ss += "]";
            }
            else{
                ss += "[]";
            }
        }
    }
    else if (is_pointer){
        int i = 0;
        while(i < ptr_level){
            ss += "*";
            i++;
        }
    }
    else if (is_func){
        switch(no_args){
            case 0: {ss += "( )";
                    break;}
            case 1: {ss += "( ";
                    auto it = arg_types.begin();
                    while(it != arg_types.end()){
                        ss += it->get_name();
                        it++;
                    }
                    ss += " )";
                    break;}
            default: {ss += "( ";
                     auto it = arg_types.begin();
                     ss += it->get_name();
                     it++;
                     while(it != arg_types.end()){
                         ss += ", ";
                         ss += it->get_name();
                         it++;
                     }
                     ss += " )";}
        }
    }

    return ss;
}

size_t Type::get_size() {
    if ( is_arr ) {
        int i=0;
        size_t size;
        unsigned int p = 1;
        while(i<arr_size){
            p *= arr_sizes[i];
            i++;
        }
        if ( !isPrimitive() ) {
            if ( defined_types[typeIndex].struct_definition != nullptr ) {
                size = defined_types[typeIndex].struct_definition->get_size();
            } else {
                error_message( "Size of " + defined_types[typeIndex].name +
                               " isn't known",
                           line_no );
                exit( 0 );
            }    
        } else {
            size = defined_types[typeIndex].size;
        }
        return size * p;
    } else if ( !isPrimitive() ) {
        if ( defined_types[typeIndex].struct_definition != nullptr ) {
            return defined_types[typeIndex].struct_definition->get_size();
        } else {
            error_message( "Size of " + defined_types[typeIndex].name +
                           " isn't known",
                       line_no );
            exit( 0 );
        }
    } else if ( ptr_level > 0  ) {
        return WORD_SIZE;
    } else if ( is_func ) {
        return WORD_SIZE;
    }
    return defined_types[typeIndex].size;
    
}

bool Type::isChar(){
    if((typeIndex == U_CHAR_T || typeIndex == CHAR_T) && ptr_level == 0)
        return true;
    return false;
}

bool Type::isInt() {
    if ( typeIndex >= U_CHAR_T && typeIndex <= LONG_T && ptr_level == 0)
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

bool Type::isVoid(){
    if(typeIndex == VOID_T)
        if(ptr_level == 0 || is_arr)
            return true;

    return false;

}

bool Type::is_invalid(){
    return typeIndex == -1 ? true : false;
}

bool Type::is_ea(){
    if(is_arr || (!isPrimitive() && ptr_level == 0)) return true;
    return false;
}


bool operator==( Type &type1, Type &type2 ) {
    if(type1.typeIndex != type2.typeIndex) return false;
    else if(type1.is_arr != type2.is_arr){
        if(type1.ptr_level == 1 && type2.ptr_level == 1) 
            return true;
        else 
            return false;
    }
    else if(type1.is_arr && type2.is_arr){
        if(type1.arr_size != type2.arr_size) return false;
        else{
            int i = 0;
            while (i < type1.arr_size){
                if(type1.arr_sizes[i] == 0 || type2.arr_sizes[i] == 0 || type1.arr_sizes[i] == type2.arr_sizes[i]){
                    i++;
                    continue;
                }
                else return false;
                i++;
            }

            return true;
        }
    }
    else if(type1.is_pointer != type2.is_pointer) return false;
    else if(type1.is_pointer && type2.is_pointer){
        return type1.ptr_level == type2.ptr_level;
    }
    else if(type1.is_func != type2.is_func) return false;
    else if(type1.is_func && type2.is_func){
        if(type1.no_args != type2.no_args) return false;
        else{
            int i = 0;
            while(i < type1.no_args){
                if(type1.arg_types[i] == type2.arg_types[i]){
                    i++;
                    continue;
                }
                else return false;
                i++;
            }
            
            return true;
        }
    }
    else return true;

    return false;
    
}

bool operator!=(Type &type1, Type &type2){
    return !(type1 == type2);
}

//############################################
//TYPES
//############################################

Types::Types()
    : index( -1 ), 
      name( "" ), 
      size( 0 ), 
      is_primitive( true ),
      is_struct( false ), 
      is_union( false ), 
      struct_definition( nullptr ){};

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

int get_type_index( string name ) {
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

StructDefinition::StructDefinition() : recursive(0){};

size_t StructDefinition::get_size() {
    if ( recursive == 1 ) {
        cout << "ERROR : Cannot recursively define struct/union type\n";
        exit( 0 );
    }
    recursive = 1;
    size_t size = 0;
    auto it = members.begin();
    while(it != members.end()){
        size_t size2 = it->second.get_size();
        if ( size2 % WORD_SIZE != 0 ) {
            size2 = size2 + ( WORD_SIZE - ( size2 % WORD_SIZE ) );
        }
        if ( un_or_st != UNION ) {
            size = size + size2;
        } else {
            size = size < size2 ? size2 : size;
        }
        it++;
    }
    assert( size != 0 );
    recursive = 0;
    return size;
}

StructDefinition *create_struct_definition( int un_or_st, StructDeclarationList *sdl ) {
    StructDefinition *sd = new StructDefinition();
    sd->un_or_st = un_or_st;
    // cout << "struct {\n";
	size_t offset = 0;
    auto it = sdl->struct_declaration_list.begin();
    while(it != sdl->struct_declaration_list.end()){
        int type_index = (*it)->sq_list->type_index;
        bool is_const = (*it)->sq_list->is_const;
        assert( type_index != -1 );
        if ( type_index == -2 ) {
            continue;
        }
        vector<Declarator *> dl =
            (*it)->declarator_list->declarator_list;
        auto it1 = dl.begin();
        while(it1 != dl.end()){
            int pointer_level = (*it1)->get_pointer_level();
            DirectDeclarator *dd = ( *it1 )->direct_declarator;
            Type type(type_index, pointer_level, is_const);
            if ( dd->type == ID ) {
                ;
            } else if ( dd->type == ARRAY ) {
                type = Type( type_index, dd->arr_sizes.size(), true );
                type.is_arr = true;
                type.is_pointer = true;
                type.arr_size = dd->arr_sizes.size();
                type.arr_sizes = dd->arr_sizes;
            }

            else if ( dd->type == FUNCTION ) {
                error_message( "Function cannot be member of struct/union",
                           ( *it1 )->id->line_no, ( *it1 )->id->col_no );
                continue;
            }
    		sd->offsets.insert( {(*it1)->id->value, offset} );
			size_t size = type.get_size();
			if ( size % WORD_SIZE != 0 ) {

				size = size + WORD_SIZE - (size % WORD_SIZE );
			}
			offset += size;
            sd->members.insert({(*it1)->id->value, type});
            it1++;
        }
            
        it++;
    }
    return sd;
}

Type *StructDefinition::get_member( Identifier *id ) {
    auto it = members.find( id->value );
    if(it != members.end()){
        return &(it->second);
    }
    return nullptr;
}

size_t StructDefinition::get_offset( Identifier * id ) {

	if ( un_or_st == UNION ) {
		return 0;
	}
	
	auto it = offsets.find(id->value);
	assert(it != offsets.end() );
	return it->second;
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
            cout << "Error in pointer type qualfier pointer ";
            cerr << "on line " << line_no << "\n";
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
    return d;
}

void Declaration::add_to_symbol_table( LocalSymbolTable &sym_tab ) {
    if ( init_declarator_list == nullptr ) {
        return;
    }
    vector<Declarator *> &dec = init_declarator_list->declarator_list;
    int type_index = declaration_specifiers->type_index;
    bool is_const = declaration_specifiers->is_const;
    assert( type_index != -1 );

    if(type_index == -2){
        // type_index error
        return;
    }

    auto it = dec.begin();
    while(it != dec.end()){
        int pointer_level = ( *it )->get_pointer_level();

        SymTabEntry *ste = new SymTabEntry(
            ( *it )->id->value, ( *it )->id->line_no, ( *it )->id->col_no );
        DirectDeclarator *dd = ( *it )->direct_declarator;

        if ( dd->type == ID ) {
            ste->type = Type( type_index, pointer_level, is_const );
            if ( ( *it )->init_expr != nullptr ) {
                //TODO: Recheck this
        		assert(0);
                #if 0
                PrimaryExpression *P = new PrimaryExpression();
                P->isTerminal = 1;
                Identifier *a = new Identifier( "" );
                *a = *( *i )->id;

                P->Ival = a;
                P->name = "primary_expression";
                P->type = Type( type_index, pointer_level, false );
                P->add_child( a );
                P->line_no = a->line_no;
                P->col_no = a->col_no;
                //P->res = new Address ( a->value, ID3);
                //Expression *ae = create_assignment_expression(
                //    P, ( *i )->eq, ( *i )->init_expr );
                //add_child( ae );
                #endif
            }
        } else if ( dd->type == ARRAY ) {
            ste->type = Type( type_index, dd->arr_sizes.size(), true );
            ste->type.is_arr = true;
            ste->type.is_pointer = true;
            ste->type.arr_size = dd->arr_sizes.size();
            ste->type.arr_sizes = dd->arr_sizes;
        } else if ( dd->type == FUNCTION ) {
            ste->type = Type( type_index, pointer_level, true );
            ste->type.is_func = true;

            //TODO: Function pointer implementation
            if ( dd->params != nullptr ) {
                ste->type.no_args = dd->params->param_list.size();
                bool fun_args_valid = true;
                
                auto jt = dd->params->param_list.begin();
                while(jt != dd->params->param_list.end()){
                    if ( ( *jt )->type.is_invalid() ) {
                        fun_args_valid = false;
                        break;
                    } else {
                        ste->type.arg_types.push_back( ( *jt )->type );
                    }
                    jt++;
                }
                
                if ( !fun_args_valid ) {
                    delete ste;
                    continue;
                }
            } else {
                ste->type.no_args = 0;
            }
        } else {
            cerr << "INVALID TYPE : " << dd->type << "\n";
            assert( 0 );
        }

        if ( ste->type.isVoid() ) {
            error_message( "Invalid type void for variable " + ( *it )->id->value, declaration_specifiers->type_specifier[0]->line_no, declaration_specifiers->type_specifier[0]->col_no );
            continue;
        }

        size_t size = ste->type.get_size();
        ste->offset = sym_tab.offset;
        if ( size % WORD_SIZE != 0 ) {
            size = size + WORD_SIZE - (size % WORD_SIZE );
        }
        sym_tab.offset = sym_tab.offset + size;
        sym_tab.reqd_size = sym_tab.offset > sym_tab.reqd_size ? sym_tab.offset : sym_tab.reqd_size;
        sym_tab.add_to_table( ste, ( *it )->id , false );
        sym_tab.ss << "global,"
                   << "-," << ( *it )->id->value << "," << ste->type.get_name()
                   << ",0\n";
        
        // if ( dd->type != FUNCTION ) {
        //     mmu.globals.insert({ste->name, ste->type.get_size()});
        // }
        it++;
    }
    write_to_symtab_file( sym_tab.ss.str() );
    sym_tab.ss.clear();
    sym_tab.ss.str( std::string() );
}


void Declaration::dotify() {

    vector<Node *> &c = children;

    bool var_dotify = false;
    auto it = c.begin();
    while(it != c.end()){
        if ( dynamic_cast<AssignmentExpression *>( *it ) ) {
            var_dotify = true;
            break;
        }
        it++;
    }
    
    if ( var_dotify == true) {
        if ( is_written ) {
            is_written = 0;
            stringstream ss;
            ss << "\t" << id << " [label=\"" << name << "\"];\n";
            it = c.begin();
            while(it != c.end()){
                if ( dynamic_cast<AssignmentExpression *>( *it ) ) {
                    ss << "\t" << id << " -> " << ( *it )->id << ";\n";
                }
                it++;
            }
            file_writer( ss.str() );

            it = c.begin();
            while(it != c.end()){
                if ( dynamic_cast<AssignmentExpression *>( *it ) ) {
                    ( *it )->dotify();
                }
                it++;
            }
        }
    }
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

    vector<TYPE_SPECIFIER> ty;
    unsigned int i = 0;
    while(i < type_specifier.size()){
        ty.push_back(type_specifier[i]->type);
        i++;
    }

    sort( ty.begin(), ty.end() );

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

        } else if ( ty[0] == VOID ) {
            type_index = VOID_T;
        } else if ( ty[0] == LONG ) {
            type_index = LONG_T;
        } else if ( ty[0] == ENUM ) {
            cerr << "ENUM not supported\n";
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
        cerr << "Error in strorage class declarator on line " << line_no << "\n";
        exit( 0 );
    }
    if ( err & 2 ) {
        error_message("Invalid Type", (type_specifier.back())->line_no, (type_specifier.back())->col_no);
        // cerr << "Error in type specifier declarator on line " << line_no << "\n";
        type_index = -2;
        // exit( 0 );
    }
    if ( err & 4 ) {
        cerr << "Error in type qualifier declarator on line " << line_no << "\n";
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

Declarator ::Declarator( Pointer *p, DirectDeclarator *dd ) : Non_Terminal( "declarator" ), pointer( p ), direct_declarator( dd ), init_expr( nullptr ), eq( nullptr ){
    if ( dd == nullptr ) {
        id = nullptr;
    } else {
        assert( dd->id != nullptr );
        id = dd->id;
    }
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
#if 0
void Declarator::dotify() {
	if(is_printed){
		is_printed = 0;
		std::stringstream ss;
		if ( init_expr != nullptr ) {
		ss << "\t" << get_id() << " [label=\"" << name << "\"];\n";
			ss << "\t" << get_id() << " -> " << init_expr->id << ";\n";
		file_writer(ss.str());
	
		init_expr->dotify();
}
	}

};
#endif

Declarator *create_declarator( Pointer *pointer,
                               DirectDeclarator *direct_declarator ) {
    if ( direct_declarator == NULL ) {
        return NULL;
    }
    Declarator *d = new Declarator( pointer, direct_declarator );
    d->add_children( pointer, direct_declarator );
    return d;
}


//################################
//STRUCT VERIFY
//################################

int verify_struct_declarator( StructDeclarationList *st ) {
    int err;
    if ( st != NULL ) {
        // cout<<st->struct_declaration_list.size();
        unsigned int i = 0;
        while(i < st->struct_declaration_list.size()){
            vector<TypeSpecifier *> ts = st->struct_declaration_list[i]->sq_list->type_specifiers;
            vector<TYPE_QUALIFIER> tq =
                st->struct_declaration_list[i]->sq_list->type_qualifiers;
            
            err = 0;
            vector<TYPE_SPECIFIER> ty;
            unsigned int j = 0;
            while(j < ts.size()){
                ty.push_back(ts[j]->type);
                j++;
            }
            
            sort( ty.begin(), ty.end() );

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
                error_message("Invalid Type", (ts.back())->line_no, (ts.back())->col_no);
                // cout << "Error in type specifier struct on line " << line_no << "\n";
                // exit(0);
                return -1;
            }
            if ( err & 4 ){
                cout << "Error in type qualifier struct on line " << line_no << "\n";
                exit(0);
                return -2;
            }
            i++;
        }
        
    }
    return 0;
}
//##############################################################################
//############################ DIRECT DECLARATOR
//###############################
//##############################################################################

DirectDeclarator ::DirectDeclarator()
    : Non_Terminal( "direct_declarator" ), type( ID ), id( nullptr ), params( nullptr ){};

DirectDeclarator *create_dir_declarator_id( DIRECT_DECLARATOR_TYPE type, Identifier *id ) {
    assert( type == ID );
    DirectDeclarator *dd = new DirectDeclarator();
    dd->type = type;
    assert( id != nullptr );
    dd->id = id;
    dd->add_child( id );
    return dd;
}

#if 0
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
#endif

DirectDeclarator *append_dir_declarator_arr( DIRECT_DECLARATOR_TYPE type, DirectDeclarator *direct_declarator, Constant * const_ ){

    if(direct_declarator == nullptr) return nullptr;

    assert( type == ARRAY );
    assert( direct_declarator->id != nullptr );
    assert( direct_declarator->type == ARRAY || direct_declarator->type == ID);

    direct_declarator->name = "direct_declarator_array";
    direct_declarator->type = ARRAY;
    assert(const_ != nullptr);
    Type const_type = const_->getConstantType();
    long dim = -1;

    switch (const_type.typeIndex)
    {
    case U_CHAR_T:
    case U_SHORT_T:
    case U_INT_T:
        direct_declarator->arr_sizes.push_back(const_->val.ui);
        break;
    case U_LONG_T:
        direct_declarator->arr_sizes.push_back( const_->val.ul );
        break;
    case CHAR_T:
    case SHORT_T:
    case INT_T:
        dim = const_->val.i;
        break;
    case LONG_T:
        dim = const_->val.l;
        break;
    default:
        error_message( "Array Dimensions must be positive integral constant", const_->line_no, const_->col_no );
    }

    if ( const_type.isUnsigned() == false && dim <= 0 ) {
        error_message( "Array Dimensions must be positive integral constant", const_->line_no, const_->col_no );
    } else {
        direct_declarator->arr_sizes.push_back( dim );
    }

    direct_declarator->add_child(const_);
    return direct_declarator;
    
}

DirectDeclarator *append_dir_declarator_fun( DIRECT_DECLARATOR_TYPE type, DirectDeclarator *direct_declarator, ParameterTypeList *params ) {
    assert( type == FUNCTION );
    assert( direct_declarator != nullptr );
    assert( direct_declarator->id != nullptr );
    assert(direct_declarator->type == ID);
    direct_declarator->name = "direct_declarator_function";
    direct_declarator->type = FUNCTION;
    direct_declarator->params = params;
    direct_declarator->add_child(params);
    return direct_declarator;

}

//##########################
//DIRECT ABSTRACT DECLARATOR
//##########################

DirectAbstractDeclarator::DirectAbstractDeclarator()
    : Non_Terminal( "direct_abstract_declarator" ){};

DirectAbstractDeclarator *create_direct_abstract_declarator( Constant *_const ) {
    DirectAbstractDeclarator *dad = new DirectAbstractDeclarator();
    if(_const == nullptr){
        dad->arr_sizes.push_back(0);
        return dad;
    }
    
    Type const_type = _const->getConstantType();
    long dim = -1;
    switch(const_type.typeIndex){
        case U_CHAR_T:
        case U_SHORT_T:
        case U_INT_T:
            dad->arr_sizes.push_back(_const->val.ui);
            break;
        case U_LONG_T:
            dad->arr_sizes.push_back(_const->val.ul);
            break;
        case CHAR_T:
        case SHORT_T:
        case INT_T:
            dim = _const->val.i;
            break;
        case LONG_T:
            dim = _const->val.l;
            break;
        default:
            error_message( "Array Dimensions must be positive integral constant", _const->line_no, _const->col_no );
    }

    if(const_type.isUnsigned() == false && dim <= 0){
        error_message("Array Dimensions must be positive integral", _const->line_no, _const->col_no);
    }
    else{
        dad->arr_sizes.push_back(dim);
    }
    return dad;
}

DirectAbstractDeclarator *append_direct_abstract_declarator(DirectAbstractDeclarator *dabs, Constant *_const){
    if ( _const == nullptr ) {
        dabs->arr_sizes.push_back( 0 );
    } else {
        Type const_type = _const->getConstantType();
        long dim = -1;
        switch ( const_type.typeIndex ) {
            case U_CHAR_T:
            case U_SHORT_T:
            case U_INT_T:
                dabs->arr_sizes.push_back( _const->val.ui );
                break;
            case U_LONG_T:
                dabs->arr_sizes.push_back( _const->val.ul );
                break;
            case CHAR_T:
            case SHORT_T:
            case INT_T:
                dim = _const->val.i;
                break;
            case LONG_T:
                dim = _const->val.l;
                break;
            default:
                error_message( "Array Dimensions must be positive integral constant", _const->line_no, _const->col_no );
        }

        if ( const_type.isUnsigned() == false && dim <= 0 ) {
            error_message( "Array Dimensions must be positive integral constant", _const->line_no, _const->col_no );
        } else {
            dabs->arr_sizes.push_back( dim );
        }
    }
    return dabs;
}


#if 0
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

#endif

//##############################
//ABSTRACT DECLARATOR
//##############################

AbstractDeclarator::AbstractDeclarator( Pointer *ptr, DirectAbstractDeclarator *dabs )
    : Non_Terminal( "abstract_declarator" ), pointer( ptr ), direct_abstract_declarator( dabs ){};

int AbstractDeclarator::get_pointer_level() {
    Pointer *ptr = pointer;
    int cnt = 0;
    while ( ptr != NULL ) {
        ptr = ptr->pointer;
        cnt++;
    }
    return cnt;
}

AbstractDeclarator *create_abstract_declarator( Pointer *ptr, DirectAbstractDeclarator *dabs ) {
    AbstractDeclarator *abs = new AbstractDeclarator( ptr, dabs );
    abs->add_children( ptr, dabs );
    return abs;
}

//######################
// TYPE NAME  
//######################
TypeName::TypeName() : Non_Terminal( "type_name" ){};

TypeName *create_type_name( SpecifierQualifierList *sq_list, AbstractDeclarator *abstract_declarator ) {
    TypeName *type_name = new TypeName();
    type_name->abstract_declarator = abstract_declarator;
    type_name->sq_list = sq_list;
    sq_list->create_type();

    assert( sq_list->type_index != -1 );
    if ( sq_list->type_index == -2 ) {
        type_name->type = Type();
    }
    if ( abstract_declarator != nullptr ) {
        DirectAbstractDeclarator *dd = abstract_declarator->direct_abstract_declarator;

        if ( dd != nullptr ) {
            if ( dd->arr_sizes.size() != 0 ) {
                type_name->type = Type( sq_list->type_index, dd->arr_sizes.size(), true );
                type_name->type.is_pointer = true;
                type_name->type.is_arr = true;
                type_name->type.arr_size = dd->arr_sizes.size();
                type_name->type.arr_sizes = dd->arr_sizes;
            }
        }
        else {
            int ptr_lvl = abstract_declarator->get_pointer_level();
            type_name->type = Type( sq_list->type_index, ptr_lvl, false );
        }
    }
    else {
        type_name->type = Type( sq_list->type_index, 0, false );
    }

    return type_name;
}
//############################
//PARAMETER DECLARATION
//############################
ParameterDeclaration::ParameterDeclaration()
    : Non_Terminal( "parameter_declaration" ){};
    
void ParameterDeclaration::create_type() {
    declaration_specifiers->create_type();
    int type_index = declaration_specifiers->type_index;
    bool is_const = declaration_specifiers->is_const;
    assert( type_index != -1 );
    if ( type_index == -2 ) {
        return;
    }

    int pointer_level = 0;
    if ( declarator != nullptr ) {
        pointer_level = declarator->get_pointer_level();
        DirectDeclarator *dd = declarator->direct_declarator;

        if ( dd->type == ARRAY ) {
            type = Type( type_index, dd->arr_sizes.size(), true );
            type.is_arr = true;
            type.is_pointer = true;
            type.arr_size = dd->arr_sizes.size();
            type.arr_sizes = dd->arr_sizes;
        } else if ( dd->type == ID ) {
            type = Type( type_index, pointer_level, is_const );
            
        }

        else if ( dd->type == FUNCTION ) {
            error_message( "Formal Argument can't be of type function", dd->id->line_no, dd->id->col_no );
            type = Type();
        }

        else {
            std::cerr << "INVALID TYPE: " << dd->type << "\n";
            assert( 0 );
        }
    } else if ( abstract_declarator != nullptr ) {
        DirectAbstractDeclarator *dd =
            abstract_declarator->direct_abstract_declarator;

        if ( dd!= nullptr ){
            if( dd->arr_sizes.size() != 0 ) {
                type = Type( type_index, dd->arr_sizes.size(), true );
                type.is_arr = true;
                type.is_pointer = true;
                type.arr_size = dd->arr_sizes.size();
                type.arr_sizes = dd->arr_sizes;
            }
        }

        else {
            pointer_level = abstract_declarator->get_pointer_level();
            type = Type( type_index, pointer_level, is_const );
        }
    } else {
        type = Type( type_index, 0, is_const );
        type.is_arr = false;
        type.is_pointer = false;
    }
};

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

Identifier::Identifier( const char *name ) : Terminal( "IDENTIFIER", name, 0, 0 ){};

Identifier::Identifier( const char *name, int _line_no,
                        int _col_no )
    : Terminal( "IDENTIFIER", name, _line_no, _col_no ){};


//###############################
// FUNCTION DEFINITION
//###############################

FunctionDefinition::FunctionDefinition(
    DeclarationSpecifiers *declaration_specifiers_, Declarator *declarator_, Node *compound_statement_ )
    : Non_Terminal( "function_definiton" ),
      declaration_specifiers( declaration_specifiers_ ),
      declarator( declarator_ ), compound_statement( compound_statement_ ), error(0){};


//################################
// TYPE SPECIFIER
//################################

TypeSpecifier::TypeSpecifier( TYPE_SPECIFIER typ, int line_no, int col_no )
    : Terminal( "type_specifier",  NULL, line_no, col_no ), type( typ ), id( nullptr ),
      struct_declaration_list( nullptr ), enumerator_list( nullptr ),
      type_index( -1 ){};

TypeSpecifier::TypeSpecifier( TYPE_SPECIFIER type_, Identifier *id_,
                              StructDeclarationList *struct_declaration_list_ )
    : Terminal( "type_specifier", NULL ), type( type_ ), id( id_ ),
      struct_declaration_list( struct_declaration_list_ ),
      enumerator_list( nullptr ), type_index( -1 ){};

TypeSpecifier::TypeSpecifier( TYPE_SPECIFIER type_, Identifier *id_,
                              EnumeratorList *enumerator_list_ )
    : Terminal( "type_specifier", NULL ), type( type_ ), id( id_ ),
      struct_declaration_list( nullptr ), enumerator_list( enumerator_list_ ),
      type_index( -1 ){};

TypeSpecifier *create_type_specifier( TYPE_SPECIFIER type, int line_no, int col_no ) {
    assert( type != UNION && type != STRUCT && type != ENUM );
    TypeSpecifier *ts = new TypeSpecifier( type, line_no, col_no );
    string ss;
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


#if 0
void verify_struct_declarator( StructDeclarationList *st ) {
    int err;
    if ( st != NULL ) {
        // std::cout<<st->struct_declaration_list.size();
        for ( int i = 0; i < st->struct_declaration_list.size(); i++ ) {
            std::vector<TypeSpecifier *> ts =
                st->struct_declaration_list.at( i )->sq_list->type_specifiers;
            std::vector<TYPE_QUALIFIER> tq =
                st->struct_declaration_list.at( i )->sq_list->type_qualifiers;
            err = 0;
            std::vector<TYPE_SPECIFIER> ty;
            for ( int i = 0; i < ts.size(); i++ )
                ty.push_back( ts.at( i )->type );
            std::sort( ty.begin(), ty.end() );

            if ( ty.size() == 3 ) {
                if ( ( ty.at( 0 ) == UNSIGNED || ty.at( 0 ) == SIGNED ) &&
                     ( ty.at( 1 ) == SHORT || ty.at( 1 ) == LONG ) &&
                     ty.at( 2 ) == INT ) {
                } else
                    err += 2;
            } else if ( ty.size() == 2 ) {
                if ( ( ty.at( 0 ) == UNSIGNED || ty.at( 0 ) == SIGNED ) &&
                     ( ty.at( 1 ) == SHORT || ty.at( 1 ) == LONG ||
                       ty.at( 1 ) == INT || ty.at( 1 ) == CHAR ) ) {
                } else if ( ( ty.at( 0 ) == SHORT || ty.at( 0 ) == LONG ) &&
                            ty.at( 1 ) == INT ) {
                } else if ( ty.at( 0 ) == LONG && ty.at( 1 ) == DOUBLE ) {
                } else
                    err += 2;
            } else if ( ty.size() == 1 )
                if ( ty.at( 0 ) == SHORT || ty.at( 0 ) == LONG ||
                     ty.at( 0 ) == INT || ty.at( 0 ) == CHAR ||
                     ty.at( 0 ) == FLOAT || ty.at( 0 ) == DOUBLE ||
                     ty.at( 0 ) == STRUCT || ty.at( 0 ) == UNION ||
                     ty.at( 0 ) == ENUM) {
                } else
                    err += 2;

            for ( int i = 0; i < tq.size(); i++ ) {
                if ( tq.at( i ) == CONST || tq.at( i ) == VOLATILE ) {
                } else {
                    err += 4;
                    break;
                }
            }
            if ( err & 2 )
                std::cout << "Error in type specifier struct";
            if ( err & 4 )
                std::cout << "Error in type qualifier struct";
        }
        // std::cout<<"done2 ";
    }
}

#endif

TypeSpecifier *
create_struct_type( TYPE_SPECIFIER type, Identifier *id ) {
    assert( type == UNION || type == STRUCT );
    TypeSpecifier *ts = new TypeSpecifier( type, id, (StructDeclarationList *) NULL );
    string ss;
    string name;
    switch ( type ) {
    case UNION:
        ss += "UNION";
        name = "union";
        break;
    case STRUCT:
        ss += "STRUCT";
        name = "struct";
        break;
    default:
        assert( 0 );
    }
    ss += " ";
    ss += id->value;
    ts->name = "type_specifier : " + ss;
    
    Types *struct_type = new Types;
    struct_type->is_primitive = false;
    struct_type->name = name + " " + id->value;

    switch ( type ) {
    case UNION:
        struct_type->is_union = true;
        struct_type->is_struct = false;
        break;
    case STRUCT:
        struct_type->is_union = false;
        struct_type->is_struct = true;
        break;
    default:
        assert( 0 );
    }

    ts->type_index = add_to_defined_types( struct_type );
    return ts;
}

TypeSpecifier * add_struct_declaration( TypeSpecifier * ts, StructDeclarationList *struct_declaration_list){
    if(struct_declaration_list != nullptr){
        int error = verify_struct_declarator(struct_declaration_list);
        if(error == 0){
            StructDefinition * struct_definition = create_struct_definition(ts->type, struct_declaration_list);
            add_struct_defintion_to_type(ts->type_index, struct_definition);
        }
        delete struct_declaration_list;
    }
    return ts;
}

TypeSpecifier *create_type_specifier( TYPE_SPECIFIER type, Identifier *id, EnumeratorList *enumerator_list ) {
    assert( type == ENUM );
    TypeSpecifier *ts = new TypeSpecifier( type, id, enumerator_list );
    string ss;
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

    vector<TYPE_SPECIFIER> ty;
    unsigned int i = 0;
    while(i < type_specifiers.size()){
        ty.push_back( type_specifiers[i]->type );
        i++;
    }


    sort( ty.begin(), ty.end() );

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
            cerr << "ENUM not supported\n";
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
        cerr << "Error in strorage class declarator on line " << line_no << "\n";
        exit( 0 );
    }
    if ( err & 2 ) {
        error_message("Invalid type", (type_specifiers.back())->line_no, (type_specifiers.back())->col_no);
        cerr << "Error in type specifier declarator on line " << line_no << "\n";
        exit( 0 );
    }
    if ( err & 4 ) {
        cerr << "Error in type qualifier declarator on line " << line_no << "\n";
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

SymbolTable::SymbolTable() : symbol_id(0) {};

void SymbolTable::add_to_table( SymTabEntry * ) { assert( 0 );}

SymTabEntry *SymbolTable::get_symbol_from_table( string name ) {
    return nullptr;
}

LocalSymbolTable::LocalSymbolTable() : current_level( 0 ), offset(0), reqd_size(0), return_type(invalid_type) {};
void LocalSymbolTable::increase_level() {
    for ( int i = 0; i < current_level; i++ ) {
        ; // cout << "  ";
    }
    // cout << "{\n";
    current_level++;
    write_to_symtab_file( ss.str() );
    ss.clear();
    ss.str( string() );
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
            //Reduce the offset to the min offset in this level;
            offset = offset > entry->offset ? entry->offset : offset;
            it->second.pop_front();
        }
        it++;
    }
    
    current_level--;
    
    write_to_symtab_file( ss.str() );
    ss.clear();
    ss.str( string() );
}

void LocalSymbolTable::add_to_table( SymTabEntry *symbol, Identifier *id , bool is_arg ) {

    auto it = sym_table.find( symbol->name );
    if ( it == sym_table.end() ) {
        deque<SymTabEntry *> &q = *new deque<SymTabEntry *>;
        symbol->level = current_level;
        if ( is_arg ) {
		symbol->id = (FUN_ARG_MASK | symbol_id++);
	    } else {
		symbol->id = (LOCAL_SYMBOL_MASK | symbol_id++);
        }
        q.push_front( symbol );
        sym_table.insert( {symbol->name, q} );
        // CSV
        ss << "local," << function_name << "," << id->value << ","
           << symbol->type.get_name() << "," << symbol->offset << "\n";
    } else {
        deque<SymTabEntry *> &q = it->second;
        if ( q.front() && ( q.front() )->level == current_level ) {
            error_message( "Redeclaration of symbol " + it->first + " in this scope", id->line_no, id->col_no );
        } else {
            symbol->level = current_level;
            symbol->id = (LOCAL_SYMBOL_MASK | symbol_id++);			
            q.push_front( symbol );
            ss << "local," << function_name << "," << id->value << ","
               << symbol->type.get_name() << "," << symbol->offset << "\n";
        }
    }
}

SymTabEntry *LocalSymbolTable::get_symbol_from_table( string name ) {
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
    DeclarationSpecifiers *declaration_specifiers, Declarator *declarator, int * error ) {

    sym_table.clear();
    symbol_id = 0;
    reqd_size = 0;
    offset = 0;
    ss.clear();
    ss.str( string() );
    if (*error == -1) return;
    assert( declarator->direct_declarator->type == FUNCTION );
    function_name = declarator->id->value;
    
    SymTabEntry *gte = global_symbol_table.get_symbol_from_table( function_name );
    if ( gte == nullptr ) {
        *error = -1;
        return;
    }
	return_type = gte->type;
	return_type.is_func = false;
    
    if ( declarator->direct_declarator->params == nullptr ) return;
    
    vector<ParameterDeclaration *> &param_list =
        declarator->direct_declarator->params->param_list;

    if ( param_list.size() == 1 ) {
        auto it = param_list.begin();
        if ( ( *it )->declarator == nullptr &&
             ( *it )->abstract_declarator == nullptr ) {
            vector<TypeSpecifier *> &v =
                ( *it )->declaration_specifiers->type_specifier;
            if ( v.size() == 1 && ( *v.begin() )->type == VOID ) {
                return;
            }
        }
    }

    current_level = 1;
    auto it = param_list.begin();
    while(it != param_list.end()){
        int type_index = ( *it )->declaration_specifiers->type_index;
        assert( type_index != -1 );
        if ( type_index == -2 ) continue;
        TypeSpecifier *ts = *(( *it )->declaration_specifiers->type_specifier.begin());

        if ( ( *it )->declarator == nullptr || ( *it )->declarator->id == nullptr ) {
            error_message( "Formal argument requires identifier", ts->line_no, ts->col_no + ts->name.size() );
            continue;
        }

        Type t = ( *it )->type;
        if ( t.isVoid() ) {
            error_message( "Invalid type void for argument " + ( *it )->declarator->id->value, ts->line_no, ts->col_no );
            continue;
        }

        SymTabEntry *symbol = new SymTabEntry(
                                                ( *it )->declarator->id->value, 
                                                ( *it )->declarator->id->line_no,
                                                ( *it )->declarator->id->col_no 
                                            );

        symbol->type = t;
        add_to_table( symbol, ( *it )->declarator->id , true );
    }
    current_level = 0;
    write_to_symtab_file( ss.str() );
    ss.clear();
    ss.str( string() );
}

//#################################
// GLOBAL SYMBOL TABLE
//#################################

void GlobalSymbolTable::add_symbol(
    DeclarationSpecifiers *declaration_specifiers, Declarator *declarator, int *error ) {

    declaration_specifiers->create_type();
    int type_index = declaration_specifiers->type_index;
    // bool is_const = declaration_specifiers->is_const;
    int pointer_level = declarator->get_pointer_level();

    assert(type_index != -1);
    if(type_index == -2){
        *error = -1;
        return;
    }
    assert(declarator->direct_declarator->type == FUNCTION);
    
    SymTabEntry *ste = new SymTabEntry(declarator->id->value, declarator->id->line_no, declarator->id->col_no);

    ste->type = Type(type_index, pointer_level, true);
    ste->type.is_func = true;
    
    DirectDeclarator * dd = declarator->direct_declarator;
    if(dd->params != nullptr){
        ste->type.no_args = dd->params->param_list.size();
        bool fun_args_are_valid = true;
        auto jt = dd->params->param_list.begin();
        while(jt != dd->params->param_list.end()){
            if((*jt)->type.is_invalid()){
                fun_args_are_valid = false;
                break;
            }
            else{
                ste->type.arg_types.push_back((*jt)->type);
            }
            jt++;
        }

        if(!fun_args_are_valid){
            delete ste;
            *error = -1;
            return;
        }
    }
    else{
        ste->type.no_args = 0;
    }

    ste->id = (GLOBAL_SYMBOL_MASK | symbol_id++);
    add_to_table(ste, true, declarator->id);
    
    // SymTabEntry *e = new SymTabEntry( declarator->id->value );
    // e->type = Type( type_index, pointer_level, is_const );
    // add_to_table( e, false );
    ss << "global, "
       << "-, " << declarator->id->value << ", fun: " << ste->type.get_name()
       << ", 0\n";

    write_to_symtab_file( ss.str() );
    ss.clear();
    ss.str( string() );

    // cout << "G: " << declarator->id->value << "\n";
}

SymTabEntry *GlobalSymbolTable::get_symbol_from_table( string name ) {
    auto it = sym_table.find( name );
    if ( it == sym_table.end() ) {
        return nullptr;
    } else {
        return it->second;
    }
}

void GlobalSymbolTable::add_to_table( SymTabEntry *symbol, bool redef, Identifier* id ) {

    auto it = sym_table.find( symbol->name );
    if ( it == sym_table.end() ) {
        symbol->id = (GLOBAL_SYMBOL_MASK | symbol_id++);

        sym_table.insert( {symbol->name, symbol} );
    } else if ( !(it->second->type == symbol->type) ) {

        // two symbols with same name can't exist at same level
        error_message("Conflicting types for " + it->first, id->line_no, id->col_no);
        // cout << "\nERROR: Redeclaration of symbol " << it->first
        //           << " on line:" << line_no << " in the scope.\n";
        // exit( 1 );
    }
}


void write_to_symtab_file( string s ) { symbol_stream << s; }

//##############################
// SYMBOL TABLE ENTRY
//##############################

SymTabEntry::SymTabEntry( string name_parameter, int line_no, int col_no ) : name( name_parameter ), line_no( line_no ), col_no( col_no ), offset(0) {};



//##############################
// ERROR LOGGING
//##############################


void error_messsage(string str, int ln_no, int col_no){
    if(col_no <= 0){
        error_message(str, ln_no);
        return;
    }

    if(ln_no == (int) (code.size() + 1) || ln_no == 0){
        cout << "\nLine: " << line_no << ":" << col_no << " \033[1;31mERROR:\033[0m " << str << "\n" << "\t" << text.str();
    }
    else{
        cout << "\nLine: " << ln_no << ":" << col_no << " \033[1;31mERROR:\033[0m " << str << "\n" << "\t" << code[ln_no - 1];
    }
    error_flag = 1;

    printf("\033[1;31m\n\t%*s\n\033[0m", col_no, "^");
}

void error_message(string str, int ln_no){
    if(ln_no == (int) (code.size() + 1) || ln_no == 0){
        cout << "\nLine: " << line_no << " \033[1;31mERROR:\033[0m " << str << "\n" << "\t" << text.str();
    }
    else{
        cout << "\nLine: " << ln_no << " \033[1;31mERROR:\033[0m " << str << "\n" << "\t" << code[ln_no - 1];
    }
    cout << "\n";
    error_flag = 1;
}

void warning_message(string str, int ln_no, int col_no){
    if(col_no <= 0){
        warning_message(str, ln_no);
        return;
    }

    if ( ln_no == (int) ( code.size() + 1 ) || ln_no == 0) {
        cout << "\nLine: " << line_no << ":" << col_no << " \033[1;35mWARNING:\033[0m " << str << "\n" << "\t" << text.str();
    } 
    else {
        cout << "\nLine: "
                << ln_no << ":" << col_no << " \033[1;35mWARNING:\033[0m " << str << "\n" << "\t" << code[ln_no - 1];
    }

    printf( "\033[1;35m\n\t%*s\n\033[0m", col_no, "^" );
}


void warning_message( string str, int ln_no ) {

    if ( ln_no == (int)( code.size() + 1 ) || ln_no == 0) {

	    cout << "\nLine: " << line_no << " \033[1;35mWARNING:\033[0m " << str << "\n" <<  "\t" << text.str();
    } 
    else {
	    cout << "\nLine: " << ln_no << " \033[1;35mWARNING:\033[0m " << str << "\n" << "\t" << code[ln_no - 1];
    }
    std::cout << "\n";

}