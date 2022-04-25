#include <algorithm>
#include <assert.h>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <vector>
#include <utility>
#include <iterator>

#include "ast.h"
#include "symtab.h"
#include "expression.h"
#include "statement.h"
#include "y.tab.h"

using namespace std;

vector<Types> defined_types;

GlobalSymbolTable global_symbol_table;
LocalSymbolTable local_symbol_table;

extern unsigned int line_no;


string primitive_type_name( PrimitiveTypes type ) {
    switch ( type ) {
        case U_CHAR_T: return "unsigned char";
        case CHAR_T: return "char";
        case U_SHORT_T: return "unsigned short";
        case SHORT_T: return "short";
        case U_INT_T: return "unsigned int";
        case INT_T: return "int";
        case U_LONG_T: return "unsigned long";
        case LONG_T: return "long";
        case FLOAT_T: return "float";
        case DOUBLE_T: return "double";
        case LONG_DOUBLE_T: return "long double";
        case VOID_T: return "void";
        default:
            cerr << "Invalid Type " << type << "\n";
            assert( 0 );
    }
}
size_t primitive_type_size( PrimitiveTypes type ) {
    switch ( type ) {
        case U_CHAR_T: return 1;
        case CHAR_T: return 1;
        case U_SHORT_T: return 2;
        case SHORT_T: return 2;
        case U_INT_T: return 4;
        case INT_T: return 4;
        case U_LONG_T: return 8;
        case LONG_T: return 8;
        case FLOAT_T: return 4;
        case DOUBLE_T: return 8;
        case LONG_DOUBLE_T: return 16;
        case VOID_T: return 0;
        default:
            cerr << "Invalid Type";
            assert( 0 );
    }
}

void instantiate_primitive_types() {
    int type = 0;
    while(type <= VOID_T){
        Types t;
        t.is_primitive = true;
        t.is_union = false;
        t.is_struct = false;
        t.index = type;
        t.name = primitive_type_name( (PrimitiveTypes)type );
        t.size = primitive_type_size( (PrimitiveTypes)type );
        t.struct_definition = nullptr;
        defined_types.push_back( t );
        type++;
    }
}

// ####################  TYPE 

Type::Type() {
    typeIndex = -1;
    ptr_level = -1;
    is_const = false;

    is_pointer = false;
    is_arr = false;
    arr_size = 0;

    is_function = false;
    num_args = 0;
}

Type::Type( int idx, int p_lvl, bool is_con ) {
    typeIndex = idx;
    is_const = is_con;

    ptr_level = p_lvl;
    is_pointer = ptr_level > 0 ? true : false;

    is_arr = false;
    arr_size = 0;

    is_function = false;
    num_args = 0;
    is_defined = false;
}
bool Type::isPrimitive(){
    return (typeIndex >= 0 && typeIndex <= VOID_T) ? true : false;
}

string Type::get_name() {
    string ss;
    ss = defined_types[typeIndex].name;

    if ( is_pointer ) {
        int i = 0;
        while(i < ptr_level){
            ss += "*";
            i++;
        }
    } else if ( is_arr ) {
        ss += " ";
        unsigned int i = 0;
        while(i < arr_size){
            ss += (arr_sizes[i]) ? ("[" + to_string(arr_sizes[i]) + "]") : "[]";
            i++;
        }
    } else if ( is_function ) {
        if ( num_args == 0 ) {
            ss += "( )";
        } else {
            ss += "( ";
            int counter = 0;
            for ( auto& it : arg_types ){
                counter += 1;
                ss += (counter == 1) ? it.get_name() : (", " + it.get_name()); 
            }
            ss += " )";
        }
    }
    return ss;
}

size_t Type::get_size() {
    if ( is_arr ) {
        unsigned int p = 1;
        for ( auto& x: arr_sizes) p *= x;

        if ( isPrimitive() ) {
            return p * defined_types[typeIndex].size;
        } else {
            if ( defined_types[typeIndex].struct_definition != nullptr ) {
                return p * defined_types[typeIndex].struct_definition->get_size();
            } else {
                error_message( "Size of type " + defined_types[typeIndex].name + " isn't known", line_no );
                exit( 0 );
            }
        }
    } else if ( ptr_level > 0 || is_function ) {
        return WORD_SIZE;
    } else if ( !isPrimitive() ) {
        if ( defined_types[typeIndex].struct_definition != nullptr ) {
            return defined_types[typeIndex].struct_definition->get_size();
        } else {
            error_message( "Size of " + defined_types[typeIndex].name + " isn't known", line_no );
            exit( 0 );
        }
    }

    return defined_types[typeIndex].size;
}

bool Type::isInt() {
    return (typeIndex >= U_CHAR_T && typeIndex <= LONG_T && ptr_level == 0);
}
bool Type::isChar() {
    return ((typeIndex == U_CHAR_T || typeIndex == CHAR_T) && ptr_level == 0);
}
bool Type::isFloat() {
    return (typeIndex >= 8 && typeIndex <= 10 && ptr_level == 0);
}
bool Type::isIntorFloat() {
    return (typeIndex <= 10 && ptr_level == 0);
}
bool Type::isUnsigned() {
    return (typeIndex == 0 || typeIndex == 2 || typeIndex == 4 || typeIndex == 6);
}
bool Type::isPointer() {
    return (ptr_level != 0);
}
void Type::make_signed() {
    typeIndex += (typeIndex == 0 || typeIndex == 2 || typeIndex == 4 || typeIndex == 6) ? 1 : 0;
}
void Type::make_unsigned() {
    typeIndex -= ( typeIndex == 1 || typeIndex == 3 || typeIndex == 5 || typeIndex == 7 ) ? 1 : 0;
}
bool Type::isVoid() {
    return (typeIndex == VOID_T && (ptr_level == 0 || is_arr));
}
bool Type::is_invalid() {
    return (typeIndex == -1);
}
bool Type::is_ea() {
	return (is_arr || (!isPrimitive() && ptr_level == 0));
}

bool operator!=( Type &obj1, Type &obj2 ) {
	return !(obj1 == obj2);
}
bool operator==( Type &obj1, Type &obj2 ) {
    if ( obj1.typeIndex != obj2.typeIndex ) {
        return false;
    } else if ( obj1.is_arr && obj2.is_arr ) {
        if ( obj1.arr_size != obj2.arr_size ) {
            return false;
        } else {
            for ( unsigned int i = 0; i < obj1.arr_size; i++ )
                if (!( obj1.arr_sizes[i] == 0 || obj2.arr_sizes[i] == 0 || obj1.arr_sizes[i] == obj2.arr_sizes[i] ))
                    return false;
            return true;
        }
    } else if ( obj1.is_arr != obj2.is_arr ) {
        return (obj1.ptr_level == 1 && obj2.ptr_level == 1);
    } else if ( obj1.is_pointer && obj2.is_pointer ) {
        return (obj1.ptr_level == obj2.ptr_level);
    } else if ( obj1.is_pointer != obj2.is_pointer ) {
        return false;
    } else if ( obj1.is_function && obj2.is_function ) {
        if ( obj1.num_args != obj2.num_args ) {
            return false;
        } else {
            for ( unsigned int i = 0; i < obj1.num_args; i++ )
                if ( obj1.arg_types[i] != obj2.arg_types[i] )
                    return false;

            return true;
        }

    } else if ( obj1.is_function != obj2.is_function ) {
        return false;
    }
    else return true;
    return false;
}

//############################## TYPES 

Types::Types()
    : is_primitive( true ), is_struct( false ), is_union( false ), index( -1 ), name( "" ), size( 0 ), struct_definition( nullptr ){};

int add_to_defined_types( Types *typ ) {
    for( auto& it : defined_types){
        if( it.name == typ->name){
            return it.index;
        }
    }

    typ->index = defined_types.size();
    defined_types.push_back( *typ );
    return defined_types.size() - 1;
}


//############################ STRUCT DEFINITION 

StructDefinition::StructDefinition() : recursive( 0 ){};

size_t StructDefinition::get_size() {
    if ( recursive == 1 ) {
        cout << "ERROR : Cannot recursively define struct/union type\n";
        exit( 0 );
    }

    recursive = 1;
    size_t size = 0;
    for ( auto& it: members ){
        size_t sz = it.second.get_size();
        if ( sz % WORD_SIZE != 0 )
            sz += ( WORD_SIZE - ( sz % WORD_SIZE ) );

        if ( un_or_st == UNION )
            size = size < sz ? sz : size;
        else
            size += sz;

    }
    assert( size != 0 );
    recursive = 0;
    return size;
}

StructDefinition *create_struct_definition( int un_or_st, StructDeclarationList *sdl ) {
    StructDefinition *sd = new StructDefinition();
    sd->un_or_st = un_or_st;
	size_t offset = 0;

    for ( auto& it: sdl->struct_declaration_list ) {
        bool is_const = it->sq_list->is_const;
        int type_index = it->sq_list->type_index;
        
        assert( type_index != -1 );
        if ( type_index == -2 ) {
            continue;
        }

        vector<Declarator*> dl = it->declarator_list->declarator_list;

        for ( auto& jt : dl ) {
            int pointer_level = jt->get_pointer_level();
            DirectDeclarator *dd = jt->direct_declarator;
            Type type( type_index, pointer_level, is_const );
            
            if ( dd->type == ARRAY ) {
                type = Type( type_index, dd->arr_sizes.size(), true );
                type.is_arr = true;
                type.is_pointer = true;
                type.arr_size = dd->arr_sizes.size();
                type.arr_sizes = dd->arr_sizes;
            }
            else if ( dd->type == FUNCTION ) {
                error_message( "Function cannot be member of struct/union", jt->id->line_no, jt->id->col_no );
                continue;
            }
			sd->offsets.insert( {jt->id->value, offset} );
			size_t size = type.get_size();
			if ( size % WORD_SIZE != 0 ) {
				size = size + WORD_SIZE - (size % WORD_SIZE );
			}
			offset += size;
            sd->members.insert( {jt->id->value, type} );
        }
    }
    return sd;
}

Type *StructDefinition::get_member( Identifier *id ) {
    auto it = members.find( id->value );
    return (it == members.end()) ? nullptr : &(it->second);
}

size_t StructDefinition::get_offset( Identifier * id ) {
	if ( un_or_st == UNION ) {
		return 0;
	}
	auto it = offsets.find(id->value);
	assert(it != offsets.end());
	return it->second;
}

//################################ POINTER

Pointer::Pointer() : Non_Terminal( "pointer" ){};

Pointer::Pointer( TypeQualifierList *type_list, Pointer *pointer_ )
    : Non_Terminal( "pointer" ), type_qualifier_list( type_list ), pointer( pointer_ ){};

Pointer *create_pointer() {
    Pointer *p = new Pointer();
    return p;
}

Pointer *create_pointer( TypeQualifierList *type_list, Pointer *pointer ) {
    Pointer *p = new Pointer( type_list, pointer );
    p->add_children( type_list, pointer );
    return p;
}

//########################### TYPE QUALIFIER LIST

TypeQualifierList ::TypeQualifierList()
    : Non_Terminal( "type_qualifier_list" ){};

TypeQualifierList ::~TypeQualifierList() { type_qualifier_list.clear(); };

void TypeQualifierList ::append_to_list( TYPE_QUALIFIER type ) {
    type_qualifier_list.push_back( type );
}

TypeQualifierList *create_type_qualifier_list( TYPE_QUALIFIER type ) {
    TypeQualifierList *tql = new TypeQualifierList();
    tql->append_to_list( type );
    return tql;
}

TypeQualifierList *add_to_type_qualifier_list( TypeQualifierList *tql, TYPE_QUALIFIER type ) {
    tql->append_to_list( type );
    return tql;
}

//############################# DECLARATION

Declaration *new_declaration( DeclarationSpecifiers *declaration_specifiers, DeclaratorList *init_declarator_list ) {
    Declaration *d = new Declaration( declaration_specifiers, init_declarator_list );
    d->add_children( declaration_specifiers, init_declarator_list );
    declaration_specifiers->create_type();
    return d;
}

Declaration ::Declaration( DeclarationSpecifiers *declaration_specifiers_, DeclaratorList *init_declarator_list_ )
    : Non_Terminal( "declaration" ), declaration_specifiers( declaration_specifiers_ ), init_declarator_list( init_declarator_list_ ){};

void Declaration::add_to_symbol_table( LocalSymbolTable &sym_tab ) {
    if ( init_declarator_list == nullptr ) {
        return;
    }
    vector<Declarator *> &dec = init_declarator_list->declarator_list;
    bool is_const = declaration_specifiers->is_const;
    int type_index = declaration_specifiers->type_index;
    
    assert( type_index != -1 );
    if ( type_index == -2 ) {
        return;
    }

    for ( auto& i : dec ) {
        int pointer_level = i->get_pointer_level();

        SymTabEntry *ste = new SymTabEntry(i->id->value, i->id->line_no, i->id->col_no );
        DirectDeclarator *dd = i->direct_declarator;

        if ( dd->type == ID ) {
            ste->type = Type( type_index, pointer_level, is_const );
            if ( i->init_expr != nullptr ) {
		        PrimaryExpression *P = new PrimaryExpression();
                P->isTerminal = 1;
                Identifier *a = new Identifier( "" );
                *a = *i->id;

                P->type = Type( type_index, pointer_level, false );
                P->Ival = a;
                P->name = "primary_expression";
                P->line_no = a->line_no;
                P->col_no = a->col_no;
                P->add_child( a );
		
                size_t size = ste->type.get_size();
                ste->offset = sym_tab.offset;
                sym_tab.offset = sym_tab.offset + size + ( size % WORD_SIZE );
                sym_tab.reqd_size = sym_tab.offset > sym_tab.reqd_size ? sym_tab.offset : sym_tab.reqd_size;
                sym_tab.add_to_table( ste, i->id , false );
                
		        P->res = new_3id( ste );
                Expression *ae = create_assignment_expression(P, i->eq, i->init_expr );
                this->add_child( ae );
		        continue;
            }
        } 
        else if ( dd->type == ARRAY ) {
            ste->type = Type( type_index, dd->arr_sizes.size(), true );
            ste->type.is_arr = true;
            ste->type.is_pointer = true;
            ste->type.arr_size = dd->arr_sizes.size();
            ste->type.arr_sizes = dd->arr_sizes;
        } 
        else if ( dd->type == FUNCTION ) {
            ste->type = Type( type_index, 0, true );
            ste->type.is_function = true;

            if ( dd->params != nullptr ) {
                ste->type.num_args = dd->params->param_list.size();

                bool fun_args_valid = true;
                for ( auto& j: dd->params->param_list ) {
                    if ( j->type.is_invalid() ) {
                        fun_args_valid = false;
                        break;
                    } else {
                        ste->type.arg_types.push_back( j->type );
                    }
                }

                if ( !fun_args_valid ) {
                    delete ste;
                    continue;
                }
            } else {
                ste->type.num_args = 0;
            }
        } else {
            cerr << "INVALID TYPE: " << dd->type << "\n";
            assert( 0 );
        }

        if ( ste->type.isVoid() ) {
            error_message( "Invalid type void for variable " + i->id->value, declaration_specifiers->type_specifier[0]->line_no, declaration_specifiers->type_specifier[0]->col_no );
            continue;
        }
        size_t size = ste->type.get_size();
        ste->offset = sym_tab.offset;
        if ( size % WORD_SIZE != 0 ) {
            size = size + WORD_SIZE - (size % WORD_SIZE );
        }
        sym_tab.offset = sym_tab.offset + size;
        sym_tab.reqd_size = sym_tab.offset > sym_tab.reqd_size ? sym_tab.offset : sym_tab.reqd_size;
        sym_tab.add_to_table( ste, i->id , false );
    }
}


void Declaration::add_to_symbol_table( GlobalSymbolTable &sym_tab ) {

    if ( init_declarator_list == nullptr )
        return;
    vector<Declarator*> &dec = init_declarator_list->declarator_list;

    bool is_const = declaration_specifiers->is_const;
    int type_index = declaration_specifiers->type_index;

    assert( type_index != -1 );
    if ( type_index == -2 ) {
        return;
    }

    for ( auto& i : dec ) {
        int pointer_level = i->get_pointer_level();

        SymTabEntry *ste = new SymTabEntry( i->id->value, i->id->line_no, i->id->col_no );
        DirectDeclarator *dd = i->direct_declarator;

        if ( dd->type == ID ) {
            ste->type = Type( type_index, pointer_level, is_const );
            if ( i->init_expr != nullptr ) {
            #if 1
                assert(0);
            #else
                PrimaryExpression *P = new PrimaryExpression();
                P->isTerminal = 1;
                Identifier *a = new Identifier( "" );
                *a = *i->id;

                P->type = Type( type_index, pointer_level, false );
                P->Ival = a;
                P->name = "primary_expression";
                P->line_no = a->line_no;
                P->col_no = a->col_no;
                P->add_child( a );
                
                size_t size = ste->type.get_size();
                ste->offset = sym_tab.offset;
                if ( size % WORD_SIZE != 0 ) {
                    size = size + WORD_SIZE - (size % WORD_SIZE );
                }
                sym_tab.offset = sym_tab.offset + size;
                sym_tab.add_to_table( ste, true, i->id );
                sym_tab.ss << "global,"
                        << "-," << i->id->value << "," << ste->type.get_name()
                        << ",0\n";

                main_mem_unit.globals.insert({ste->name, ste->type.get_size()});

		        P->res = new_3id( ste );
                Expression *ae = create_assignment_expression(P, i->eq, i->init_expr );
                add_child( ae );
                continue;
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
            ste->type.is_function = true;
            if ( dd->params != nullptr ) {
                ste->type.num_args = dd->params->param_list.size();
                bool fun_args_valid = true;
                for ( auto& j : dd->params->param_list ) {
                    if ( j->type.is_invalid() ) {
                        fun_args_valid = false;
                        break;
                    } else {
                        ste->type.arg_types.push_back( j->type );
                    }
                }

                if ( !fun_args_valid ) {
                    delete ste;
                    continue;
                }
            } else {
                ste->type.num_args = 0;
            }
        } else {
            cerr << "INVALID TYPE : " << dd->type << "\n";
            assert( 0 );
        }

        if ( ste->type.isVoid()  && dd->type != FUNCTION ) {
            error_message( "Invalid type void for variable " + i->id->value,  declaration_specifiers->type_specifier[0]->line_no, declaration_specifiers->type_specifier[0]->col_no );
            continue;
        }

        size_t size = ste->type.get_size();
        ste->offset = sym_tab.offset;
        if ( size % WORD_SIZE != 0 ) {
            size = size + WORD_SIZE - (size % WORD_SIZE );
        }
        sym_tab.offset = sym_tab.offset + size;
        sym_tab.add_to_table( ste, true, i->id );
        sym_tab.ss << "global,"
                   << "-," << i->id->value << "," << ste->type.get_name()
                   << ",0\n";

        if ( dd->type != FUNCTION ) {
            main_mem_unit.globals.insert({ste->name, ste->type.get_size()});
        }
    }


    write_to_symtab_file( sym_tab.ss.str() );
    sym_tab.ss.clear();
    sym_tab.ss.str( string() );
}


void Declaration::dotify() {

    vector<Node *> &c = children;
    
    bool dotify_ = false;
    for ( auto& it: c ) {
        if ( dynamic_cast<AssignmentExpression *>(it) ) {
            dotify_ = true;
            break;
        }
    }
    if ( dotify_ ) {
        if ( is_printed ) {
            is_printed = 0;
            string ss;
            ss = "\t" + to_string(id) + " [label=\"" + name + "\"];\n";
            for ( auto& it: c ) {
                if ( dynamic_cast<AssignmentExpression *>(it) ) {
                    ss += "\t" + to_string(id) + " -> " + to_string(it->id) + ";\n";
                }
            }
            file_writer( ss );

            for ( auto& it: c ) {
                if ( dynamic_cast<AssignmentExpression *>(it) ) {
                    it->dotify();
                }
            }
        }
    }
}

//########################## DECLARATION SPECIFIERS 

DeclarationSpecifiers ::DeclarationSpecifiers()
    : Non_Terminal( "declaration_specifiers" ), is_const( false ),
      type_index( -1 ){};

void DeclarationSpecifiers::create_type() {

    int err = 0;

    err += (storage_class.size() == 0 || (storage_class.size() == 1 && storage_class[0] == TYPEDEF)) ? 0 : 1;

    vector<TYPE_SPECIFIER> ty;
    for ( auto& t: type_specifier )
        ty.push_back( t->type );

    sort( ty.begin(), ty.end() );

    is_const = false;
    type_index = -1;

    if ( ty.size() == 1 ) {
        switch(ty[0]){
            case FLOAT: type_index = FLOAT_T; break;
            case DOUBLE: type_index = DOUBLE_T; break;
            case INT: type_index = INT_T; break;
            case SHORT: type_index = SHORT_T; break;
            case CHAR: type_index = CHAR_T; break;
            case VOID: type_index = VOID_T; break;
            case LONG: type_index = LONG_T; break;
            case STRUCT:
            case UNION: type_index = type_specifier[0]->type_index; break;
            case ENUM: cerr << "ENUM not supported\n"; exit(0);
            default: err += 2;
        }
    }
    else if ( ty.size() == 2 ) {
        if ( ty[0] == UNSIGNED && ty[1] == CHAR ) {
            type_index = U_CHAR_T;
        } 
        else if ( ty[0] == SIGNED && ty[1] == CHAR ) {
            type_index = CHAR_T;
        } 
        else if ( ty[0] == UNSIGNED && ty[1] == SHORT ) {
            type_index = U_SHORT_T;
        } 
        else if ( ty[0] == SIGNED && ty[1] == SHORT ) {
            type_index = SHORT_T;
        } 
        else if ( ty[0] == UNSIGNED && ty[1] == INT ) {
            type_index = U_INT_T;
        } 
        else if ( ty[0] == SIGNED && ty[1] == INT ) {
            type_index = INT_T;
        } 
        else if ( ty[0] == UNSIGNED && ty[1] == LONG ) {
            type_index = U_LONG_T;
        } 
        else if ( ty[0] == SIGNED && ty[1] == LONG ) {
            type_index = LONG_T;
        } else
            err += 2;
    } 
    else if ( ty.size() == 3 ) {
        if ( ty[0] == UNSIGNED && ty[1] == LONG && ty[2] == LONG ) {
            type_index = U_LONG_T;
        } 
        else err += 2;
    } 
    for ( unsigned int i = 0; i < type_qualifier.size(); i++ ) {
        if ( type_qualifier[i] == CONST ) {
            is_const = true;
        }
        else {
            err += 4;
            break;
        }
    }
    if ( err & 1 ) {
        cerr << "Error in strorage class declarator at line " << line_no << "\n";
        exit( 0 );
    }
    if ( err & 2 ) {
        error_message( "Invalid type", (type_specifier.back())->line_no, (type_specifier.back())->col_no );
        type_index = -2;
    }
    if ( err & 4 ) {
        cerr << "Error in type qualifier declarator at line " << line_no << "\n";
        exit( 0 );
    }
}

DeclarationSpecifiers *new_storage_class( STORAGE_CLASS sc ) {
    DeclarationSpecifiers *ds = new DeclarationSpecifiers();
    ds->storage_class.push_back( sc );
    return ds;
}
DeclarationSpecifiers *add_storage_class( DeclarationSpecifiers *ds, STORAGE_CLASS sc ) {
    ds->storage_class.push_back( sc );
    return ds;
}

DeclarationSpecifiers *new_type_specifier( TypeSpecifier *ts ) {
    DeclarationSpecifiers *ds = new DeclarationSpecifiers();
    ds->type_specifier.push_back( ts );
    ds->add_child( ts );
    return ds;
}
DeclarationSpecifiers *add_type_specifier( DeclarationSpecifiers *ds, TypeSpecifier *ts ) {
    ds->type_specifier.push_back( ts );
    ds->add_child( ts );
    return ds;
}

DeclarationSpecifiers *new_type_qualifier( TYPE_QUALIFIER tq ) {
    DeclarationSpecifiers *ds = new DeclarationSpecifiers();
    ds->type_qualifier.push_back( tq );
    return ds;
}
DeclarationSpecifiers *add_type_qualifier( DeclarationSpecifiers *ds, TYPE_QUALIFIER tq ) {
    ds->type_qualifier.push_back( tq );
    return ds;
}

//########################### DECLARATION LIST

DeclarationList::DeclarationList() : Non_Terminal( "declaration_list" ){};

DeclarationList *create_declaration_list( Declaration *declaration ) {
    DeclarationList *dl = new DeclarationList();
    dl->declarations.push_back( declaration );
    dl->add_child( declaration );
    declaration->add_to_symbol_table( local_symbol_table );
    return dl;
}
DeclarationList *add_to_declaration_list( DeclarationList *declaration_list, Declaration *declaration ) {
    if ( declaration == nullptr ) {
        return declaration_list;
    }
    if ( declaration_list == nullptr ) {
        return create_declaration_list( declaration );
    }
    declaration_list->declarations.push_back( declaration );
    declaration->add_to_symbol_table( local_symbol_table );
    declaration_list->add_child( declaration );
    return declaration_list;
}

//############################# DECLARATOR LIST

DeclaratorList ::DeclaratorList() : Non_Terminal( "init_declarator_list" ){};

DeclaratorList *create_init_declarator_list( Declarator *d ) {
    if ( d == nullptr ) return nullptr;
    DeclaratorList *dl = new DeclaratorList();
    dl->declarator_list.push_back( d );
    dl->add_child( d );
    return dl;
}

DeclaratorList *add_to_init_declarator_list( DeclaratorList *dl, Declarator *d ) {
    if ( d == nullptr ) return dl;
    if ( dl == nullptr ) {
        dl = new DeclaratorList();
    }
    dl->declarator_list.push_back( d );
    dl->add_child( d );
    return dl;
}

//################################# DECLARATOR

Declarator::Declarator( Pointer *p, DirectDeclarator *dd )
    : Non_Terminal( "declarator" ), pointer( p ), direct_declarator( dd ), init_expr( nullptr ), eq( nullptr ) {
    if ( dd == nullptr ) id = nullptr;
    else {
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

Declarator *create_declarator( Pointer *pointer, DirectDeclarator *direct_declarator ) {
    if ( direct_declarator == NULL ) return NULL;
    Declarator *d = new Declarator( pointer, direct_declarator );
    d->add_children( pointer, direct_declarator );
    return d;
}

Declarator *add_initializer_to_declarator( Declarator *declarator, Terminal *eq, Expression *ie ) {
    if ( declarator == nullptr ) return NULL;
    
    if ( declarator->direct_declarator->type == ID ) {
        declarator->eq = eq;
        declarator->init_expr = ie;
        declarator->add_child( ie );
        return declarator;
    } 
    else if ( declarator->direct_declarator->type == ARRAY ) {
        error_message( "Unsupported initialization of array declaration", eq->line_no, eq->col_no );
        return declarator;
    } 
    else if ( declarator->direct_declarator->type == FUNCTION ) {
        error_message( "Invalid initialization of function declaration", eq->line_no, eq->col_no );
        return declarator;
    } 
    else {
        cerr << "INVALID TYPE: " << declarator->direct_declarator->type << "\n";
        assert( 0 );
    }
    return NULL;
}

//############################## STRUCT VERIFY

int verify_struct_declarator( StructDeclarationList *st ) {
    int err;
    if ( st != NULL ) {
        for ( auto& sd: st->struct_declaration_list ) {
            vector<TYPE_QUALIFIER> tq = sd->sq_list->type_qualifiers;
            vector<TypeSpecifier *> ts = sd->sq_list->type_specifiers;
            
            err = 0;
            vector<TYPE_SPECIFIER> ty;
            for( auto& i: ts){
                ty.push_back(i->type);
            }
            sort( ty.begin(), ty.end() );

            if ( ty.size() == 1 ) {
                switch(ty[0]){
                    case SHORT:
                    case LONG:
                    case INT:
                    case CHAR:
                    case FLOAT:
                    case DOUBLE:
                    case STRUCT:
                    case UNION:
                    case ENUM: break;
                    default: err += 2;
                }
            }
            else if ( ty.size() == 2 ) {
                if ( ( ty[0] == UNSIGNED || ty[0] == SIGNED ) &&
                     ( ty[1] == SHORT || ty[1] == LONG || ty[1] == INT || ty[1] == CHAR ) ) {
                } 
                else if ( ( ty[0] == SHORT || ty[0] == LONG ) && ty[1] == INT ) {
                } 
                else if ( ty[0] == LONG && ty[1] == DOUBLE ) {
                } 
                else err += 2;
            } 
            else if ( ty.size() == 3 ) {
                if ( ( ty[0] == UNSIGNED || ty[0] == SIGNED ) &&
                     ( ty[1] == SHORT || ty[1] == LONG ) &&
                     ty[2] == INT ) {
                } 
                else err += 2;
            } 

            for ( auto& i: tq ) {
                if ( i == CONST || i == VOLATILE ) {
                } 
                else {
                    err += 4;
                    break;
                }
            }

            if ( err & 2 ) {
                error_message( "Invalid type", (ts.back())->line_no, (ts.back())->col_no );
                return -1;
            }
            if ( err & 4 ) {
                cout << "Error in type qualifier struct at line " << line_no << "\n";
                exit( 0 );
                return -2;
            }
        }
    }
    return 0;
}

//############################ DIRECT DECLARATOR

DirectDeclarator ::DirectDeclarator()
    : Non_Terminal( "direct_declarator" ), type( ID ), id( nullptr ), params( nullptr ){};

DirectDeclarator *create_dir_declarator_id( DIRECT_DECLARATOR_TYPE type, Identifier *id ) {
    assert( type == ID );
    assert( id != nullptr );
    
    DirectDeclarator *dd = new DirectDeclarator();
    dd->type = type;
    dd->id = id;

    dd->add_child( id );
    return dd;
}

DirectDeclarator *append_dir_declarator_arr( DIRECT_DECLARATOR_TYPE type, DirectDeclarator *direct_declarator, Constant *const_ ) {
    if ( direct_declarator == nullptr ) return nullptr;

    assert( type == ARRAY );
    assert( direct_declarator->id != nullptr );
    assert( direct_declarator->type == ARRAY || direct_declarator->type == ID );
    assert( const_ != nullptr );

    direct_declarator->name = "direct_declarator_array";
    direct_declarator->type = ARRAY;
    Type const_type = const_->getConstantType();
    long dim = -1;
    switch ( const_type.typeIndex ) {
        case U_CHAR_T:
        case U_SHORT_T:
        case U_INT_T:
            direct_declarator->arr_sizes.push_back( const_->val.ui );
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

    if ( const_type.isUnsigned() || dim > 0 )
        direct_declarator->arr_sizes.push_back( dim );
    else
        error_message( "Array Dimensions must be positive integral constant", const_->line_no, const_->col_no );

    direct_declarator->add_child( const_ );
    return direct_declarator;
}

DirectDeclarator *append_dir_declarator_fun( DIRECT_DECLARATOR_TYPE type, DirectDeclarator *direct_declarator, ParameterTypeList *params ) {
    assert( type == FUNCTION );
    assert( direct_declarator != nullptr );
    assert( direct_declarator->id != nullptr );
    assert( direct_declarator->type == ID );

    direct_declarator->name = "direct_declarator_function";
    direct_declarator->type = FUNCTION;
    direct_declarator->params = params;
    direct_declarator->add_child( params );
    return direct_declarator;
}

//######################## DIRECT ABSTRACT DECLARATOR

DirectAbstractDeclarator::DirectAbstractDeclarator(): Non_Terminal( "direct_abstract_declarator" ){};

DirectAbstractDeclarator *create_direct_abstract_declarator( Constant *_const ) {
    DirectAbstractDeclarator *dad = new DirectAbstractDeclarator();

    if ( _const == nullptr )
        dad->arr_sizes.push_back(0);
    else {
        Type const_type = _const->getConstantType();
        long dim = -1;
        switch ( const_type.typeIndex ) {
            case U_CHAR_T:
            case U_SHORT_T:
            case U_INT_T:
                dad->arr_sizes.push_back( _const->val.ui );
                break;
            case U_LONG_T:
                dad->arr_sizes.push_back( _const->val.ul );
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

        if ( const_type.isUnsigned() || dim > 0 )
            dad->arr_sizes.push_back( dim );
        else
            error_message( "Array Dimensions must be positive integral constant", _const->line_no, _const->col_no );
    }
    return dad;
}

DirectAbstractDeclarator *append_direct_abstract_declarator( DirectAbstractDeclarator *dad, Constant *_const ) {
    if ( _const == nullptr )
        dad->arr_sizes.push_back( 0 );
    else {
        Type const_type = _const->getConstantType();
        long dim = -1;
        switch ( const_type.typeIndex ) {
            case U_CHAR_T:
            case U_SHORT_T:
            case U_INT_T:
                dad->arr_sizes.push_back( _const->val.ui );
                break;
            case U_LONG_T:
                dad->arr_sizes.push_back( _const->val.ul );
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

        if ( const_type.isUnsigned() || dim > 0 )
            dad->arr_sizes.push_back( dim );
        else
            error_message( "Array Dimensions must be positive integral constant", _const->line_no, _const->col_no );
    }
    return dad;
}

//########################### ABSTRACT DECLARATOR

AbstractDeclarator::AbstractDeclarator( Pointer *ptr, DirectAbstractDeclarator *dabs )
    : Non_Terminal( "abstract_declarator" ), pointer( ptr ), direct_abstract_declarator( dabs ){};

int AbstractDeclarator::get_pointer_level() {
    Pointer *ptr = pointer;
    int count = 0;
    while ( ptr != NULL ) {
        ptr = ptr->pointer;
        count++;
    }
    return count;
}

AbstractDeclarator *create_abstract_declarator( Pointer *ptr, DirectAbstractDeclarator *dabs ) {
    AbstractDeclarator *abs = new AbstractDeclarator( ptr, dabs );
    abs->add_children( ptr, dabs );
    return abs;
}

//############################### TYPE NAME

TypeName::TypeName() : Non_Terminal( "type_name" ){};

TypeName *create_type_name( SpecifierQualifierList *sq_list, AbstractDeclarator *abstract_declarator ) {
    TypeName *type_name = new TypeName();
    type_name->sq_list = sq_list;
    type_name->abstract_declarator = abstract_declarator;
    
    sq_list->create_type();

    assert( sq_list->type_index != -1 );

    if ( sq_list->type_index == -2 )
        type_name->type = Type();

    if ( abstract_declarator == nullptr )
        type_name->type = Type( sq_list->type_index, 0, false );
    else {
        DirectAbstractDeclarator *dd = abstract_declarator->direct_abstract_declarator;

        if ( dd != nullptr && dd->arr_sizes.size() != 0 ) {
            type_name->type = Type( sq_list->type_index, dd->arr_sizes.size(), true );
            type_name->type.is_pointer = true;

            type_name->type.is_arr = true;
            type_name->type.arr_size = dd->arr_sizes.size();
            type_name->type.arr_sizes = dd->arr_sizes;
        }
        else {
            int pointer_level = abstract_declarator->get_pointer_level();
            type_name->type = Type( sq_list->type_index, pointer_level, false );
        }
    }
    return type_name;
}

//########################### PARAMETER DECLARATION

ParameterDeclaration::ParameterDeclaration() : Non_Terminal( "parameter_declaration" ){};

void ParameterDeclaration::create_type() {
    declaration_specifiers->create_type();

    int type_index = declaration_specifiers->type_index;
    bool is_const = declaration_specifiers->is_const;

    assert( type_index != -1 );
    if ( type_index == -2 ) return;

    int pointer_level = 0;
    if (declarator != nullptr) {
        pointer_level = declarator->get_pointer_level();
        DirectDeclarator *dd = declarator->direct_declarator;

        if ( dd->type == ID ) {
            type = Type( type_index, pointer_level, is_const );
        } 
        else if ( dd->type == ARRAY ) {
            type = Type( type_index, dd->arr_sizes.size(), true );
            type.is_pointer = true;

            type.is_arr = true;
            type.arr_size = dd->arr_sizes.size();
            type.arr_sizes = dd->arr_sizes;
        }
        else if ( dd->type == FUNCTION ) {
            error_message( "Formal Argument can't be of type function", dd->id->line_no, dd->id->col_no );
            type = Type();
        }
        else {
            cerr << "INVALID TYPE: " << dd->type << "\n";
            assert( 0 );
        }
    } else if ( abstract_declarator != nullptr ) {

        DirectAbstractDeclarator *dd = abstract_declarator->direct_abstract_declarator;

        if ( dd != nullptr && dd->arr_sizes.size() != 0 ) {
            type = Type( type_index, dd->arr_sizes.size(), true );
            type.is_pointer = true;

            type.is_arr = true;
            type.arr_size = dd->arr_sizes.size();
            type.arr_sizes = dd->arr_sizes;
        }
        else {
            pointer_level = abstract_declarator->get_pointer_level();
            type = Type( type_index, pointer_level, is_const );
        }
    } else {
        type = Type( type_index, 0, is_const );
        type.is_pointer = false;
        type.is_arr = false;
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

//############################ PARAMETER TYPE LIST

ParameterTypeList::ParameterTypeList(): Non_Terminal( "parameter_type_list" ){};

ParameterTypeList *create_parameter_list( ParameterDeclaration *pd ) {
    ParameterTypeList *ptl = new ParameterTypeList();
    pd->create_type();
    ptl->param_list.push_back( pd );
    ptl->add_child( pd );
    return ptl;
}
ParameterTypeList *add_to_parameter_list( ParameterTypeList *ptl, ParameterDeclaration *pd ) {
    pd->create_type();
    ptl->param_list.push_back( pd );
    ptl->add_child( pd );
    return ptl;
}
ParameterTypeList *add_ellipsis_to_list( ParameterTypeList *ptl ) {
    ptl->has_ellipsis = true;
    ptl->add_child( create_terminal( "...", NULL ) );
    return ptl;
}

//################################ IDENTIFIER

Identifier::Identifier( const char *name ): Terminal( "IDENTIFIER", name, 0, 0 ){};

Identifier::Identifier( const char *name, unsigned int _line_no, unsigned int _col_no ): Terminal( "IDENTIFIER", name, _line_no, _col_no ){};

//########################## FUNCTION DEFINITION

FunctionDefinition::FunctionDefinition( DeclarationSpecifiers *declaration_specifiers_, Declarator *declarator_, Node *compound_statement_ )
    : Non_Terminal( "function_definiton" ), declaration_specifiers( declaration_specifiers_ ),
      declarator( declarator_ ), compound_statement( compound_statement_ ), error( 0 ){};

FunctionDefinition *create_function_defintion( DeclarationSpecifiers *declaration_specifiers, Declarator *declarator, Node *compound_statement ) {
    FunctionDefinition *fd = new FunctionDefinition( declaration_specifiers, declarator, compound_statement );
    fd->add_children( declaration_specifiers, declarator, compound_statement );
    global_symbol_table.add_symbol( declaration_specifiers, declarator, &( fd->error ) );
    local_symbol_table.add_function( declaration_specifiers, declarator, &( fd->error ) );
    return fd;
}

FunctionDefinition *add_stmt_to_function_definition( FunctionDefinition *fd, Node *compound_stmt ) {
    fd->compound_statement = compound_stmt;
    fd->add_child( compound_stmt );
    if ( fd->error == 0 && compound_stmt != nullptr ) {
        SymTabEntry *gte = global_symbol_table.get_symbol_from_table( fd->declarator->id->value );
        gte->type.is_defined = true;
    }
    return fd;
}

//############################## TYPE SPECIFIER

TypeSpecifier::TypeSpecifier( TYPE_SPECIFIER typ, unsigned int _line_no, unsigned int _col_no )
    : Terminal( "type_specifier", NULL, _line_no, _col_no ), type( typ ),
      id( nullptr ), struct_declaration_list( nullptr ),
      enumerator_list( nullptr ), type_index( -1 ){};

TypeSpecifier::TypeSpecifier( TYPE_SPECIFIER type_, Identifier *id_, StructDeclarationList *struct_declaration_list_ )
    : Terminal( "type_specifier", NULL ), type( type_ ), id( id_ ),
      struct_declaration_list( struct_declaration_list_ ),
      enumerator_list( nullptr ), type_index( -1 ){};

TypeSpecifier::TypeSpecifier( TYPE_SPECIFIER type_, Identifier *id_, EnumeratorList *enumerator_list_ )
    : Terminal( "type_specifier", NULL ), type( type_ ), id( id_ ),
      struct_declaration_list( nullptr ), enumerator_list( enumerator_list_ ),
      type_index( -1 ){};

TypeSpecifier *create_type_specifier( TYPE_SPECIFIER type, unsigned int line_no, unsigned int col_no ) {
    assert( type != UNION && type != STRUCT && type != ENUM );
    TypeSpecifier *ts = new TypeSpecifier( type, line_no, col_no );
    string ss;
    switch ( type ) {
        case UNSIGNED: ss = "UNSIGNED"; break;
        case SIGNED: ss = "SIGNED"; break;
        case CHAR: ss = "CHAR"; break;
        case SHORT: ss = "SHORT"; break;
        case INT: ss = "INT"; break;
        case LONG: ss = "LONG"; break;
        case FLOAT: ss = "FLOAT"; break;
        case DOUBLE: ss = "DOUBLE"; break;
        case VOID: ss = "VOID"; break;
        case TYPE_NAME: ss = "TYPE_NAME"; break;
        default:
            error_message("Can't create type specfier for type" + type,line_no, col_no);
            exit(0); 
            // assert( 0 );
    }
    ts->name = ss;
    return ts;
}

TypeSpecifier *create_struct_type( TYPE_SPECIFIER type, Identifier *id ) {
    assert( type == UNION || type == STRUCT );
    TypeSpecifier *ts = new TypeSpecifier( type, id, (StructDeclarationList *)NULL );
    string ss;
    string name_;
    switch ( type ) {
        case UNION: ss = "UNION"; name_ = "union"; break;
        case STRUCT: ss = "STRUCT"; name_ = "struct"; break;
        default: assert( 0 );
    }
    ss += " " + id->value;
    ts->name = "type_specifier : " + ss;

    Types *struct_type = new Types;
    struct_type->is_primitive = false;
    struct_type->name = name_ + " " + id->value;

    switch ( type ) {
        case UNION:
            struct_type->is_union = true;
            struct_type->is_struct = false;
            break;
        case STRUCT:
            struct_type->is_union = false;
            struct_type->is_struct = true;
            break;
        default: assert( 0 );
    }
    ts->type_index = add_to_defined_types( struct_type );

    return ts;
}

TypeSpecifier *add_struct_declaration( TypeSpecifier *ts, StructDeclarationList *struct_declaration_list ) {
    if ( struct_declaration_list != nullptr ) {
        int err = verify_struct_declarator( struct_declaration_list );
        if ( err == 0 ) {
            StructDefinition *struct_definition = create_struct_definition( ts->type, struct_declaration_list );
            defined_types[ts->type_index].struct_definition = struct_definition;
            defined_types[ts->type_index].size = struct_definition->get_size();
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

//######################### STRUCT DECLARATION LIST

StructDeclarationList::StructDeclarationList(): Non_Terminal( "struct_declaration_list" ){};

StructDeclarationList *create_struct_declaration_list( StructDeclaration *struct_declaration ) {
    StructDeclarationList *sdl = new StructDeclarationList();
    sdl->struct_declaration_list.push_back( struct_declaration );
    sdl->add_child( struct_declaration );
    return sdl;
}

StructDeclarationList *add_to_struct_declaration_list( StructDeclarationList *struct_declaration_list, StructDeclaration *struct_declaration ) {
    struct_declaration_list->struct_declaration_list.push_back( struct_declaration );
    struct_declaration_list->add_child( struct_declaration );
    return struct_declaration_list;
}

//############################ STRUCT DECLARATION

StructDeclaration::StructDeclaration( SpecifierQualifierList *sq_list_, DeclaratorList *declarator_list_ )
    : Non_Terminal( "struct_declaration" ), sq_list( sq_list_ ),
      declarator_list( declarator_list_ ){};

StructDeclaration *create_struct_declaration( SpecifierQualifierList *sq_list, DeclaratorList *struct_declarator_list ) {
    StructDeclaration *sd = new StructDeclaration( sq_list, struct_declarator_list );
    sd->add_children( sq_list, struct_declarator_list );
    sq_list->create_type();
    return sd;
}

//######################### SPECIFIER QUALIFIER LIST

SpecifierQualifierList::SpecifierQualifierList(): Non_Terminal( "specifier_qualifier_list" ){};

void SpecifierQualifierList::create_type() {
    int err = 0;
    vector<TYPE_SPECIFIER> ty;

    for ( auto& ts: type_specifiers )
        ty.push_back( ts->type );
    sort( ty.begin(), ty.end() );

    is_const = false;
    type_index = -1;
    if ( ty.size() == 1 ) {
        switch(ty[0]){
            case CHAR: type_index = CHAR_T; break;
            case SHORT: type_index = SHORT_T; break;
            case INT: type_index = INT_T; break;
            case LONG: type_index = LONG_T; break;
            case FLOAT: type_index = FLOAT_T; break;
            case DOUBLE: type_index = DOUBLE_T; break;
            case VOID: type_index = VOID_T; break;
            case STRUCT:
            case UNION: type_index = type_specifiers[0]->type_index; break;
            case ENUM: cerr << "ENUM not supported\n"; exit(0);
            default: err += 2;
        }
    }
    else if ( ty.size() == 2 ) {
        if ( ty[0] == UNSIGNED && ty[1] == CHAR )
            type_index = U_CHAR_T; 
        else if ( ty[0] == SIGNED && ty[1] == CHAR )
            type_index = CHAR_T;
        else if ( ty[0] == UNSIGNED && ty[1] == SHORT )
            type_index = U_SHORT_T;
        else if ( ty[0] == SIGNED && ty[1] == SHORT )
            type_index = SHORT_T;
        else if ( ty[0] == UNSIGNED && ty[1] == INT )
            type_index = U_INT_T;
        else if ( ty[0] == SIGNED && ty[1] == INT )
            type_index = INT_T;
        else if ( ty[0] == UNSIGNED && ty[1] == LONG )
            type_index = U_LONG_T;
        else if ( ty[0] == SIGNED && ty[1] == LONG )
            type_index = LONG_T;
        else
            err += 2;
    } 
    else if ( ty.size() == 3 ) {
        if ( ty[0] == UNSIGNED && ty[1] == LONG && ty[2] == LONG )
            type_index = U_LONG_T;
        else err += 2;
    } 

    for ( auto& i: type_qualifiers ) {
        if ( i == CONST ) is_const = true;
        else if ( i == VOLATILE ) ;
        else {
            err += 4;
            break;
        }
    }

    if ( err & 1 ) {
        cerr << "Error in strorage class declarator at line " << line_no << "\n";
        exit( 0 );
    }
    if ( err & 2 ) {
        error_message( "Invalid type", (type_specifiers.back())->line_no, (type_specifiers.back())->col_no );
        type_index = -2;
    }
    if ( err & 4 ) {
        cerr << "Error in type qualifier declarator at line " << line_no << "\n";
        exit( 0 );
    }
}

SpecifierQualifierList *create_type_specifier_sq( TypeSpecifier *type_specifier ) {
    SpecifierQualifierList *sql = new SpecifierQualifierList();
    sql->type_specifiers.push_back( type_specifier );
    sql->add_child( type_specifier );
    return sql;
}
SpecifierQualifierList *add_type_specifier_sq( SpecifierQualifierList *sq_list, TypeSpecifier *type_specifier ) {
    sq_list->type_specifiers.push_back( type_specifier );
    sq_list->add_child( type_specifier );
    return sq_list;
}

SpecifierQualifierList *create_type_qualifier_sq( TYPE_QUALIFIER type_qualifier ) {
    SpecifierQualifierList *sql = new SpecifierQualifierList();
    sql->type_qualifiers.push_back( type_qualifier );
    return sql;
}
SpecifierQualifierList *add_type_qualifier_sq( SpecifierQualifierList *sq_list, TYPE_QUALIFIER type_qualifier ) {
    sq_list->type_qualifiers.push_back( type_qualifier );
    return sq_list;
}

//################################ ENUMERATOR

Enumerator::Enumerator( Identifier *id_, Node *const_expr )
    : Non_Terminal( "enumerator" ), id( id_ ), init_expr( const_expr ){};

Enumerator *create_enumerator( Identifier *id, Node *const_expr ) {
    Enumerator *ste = new Enumerator( id, const_expr );
    ste->add_children( id, const_expr );
    return ste;
}

//############################## ENUMERATOR LIST

EnumeratorList::EnumeratorList() : Non_Terminal( "enumerator_list" ){};

EnumeratorList *create_enumerator_list( Enumerator *enumerator ) {
    EnumeratorList *el = new EnumeratorList();
    el->enumerator_list.push_back( enumerator );
    el->add_child( enumerator );
    return el;
}
EnumeratorList *add_to_enumerator_list( EnumeratorList *enumerator_list, Enumerator *enumerator ) {
    enumerator_list->enumerator_list.push_back( enumerator );
    enumerator_list->add_child( enumerator );
    return enumerator_list;
}

//############################ LOCAL SYMBOL TABLE

SymbolTable::SymbolTable() : symbol_id(0) {};

void SymbolTable::add_to_table( SymTabEntry * ) { assert( 0 ); }

SymTabEntry *SymbolTable::get_symbol_from_table( string name ) {
    return nullptr;
}

LocalSymbolTable::LocalSymbolTable() : reqd_size( 0 ), current_level(0), offset(0), return_type(invalid_type) {};

void LocalSymbolTable::increase_level() {
    current_level++;
    write_to_symtab_file( ss.str() );
    ss.clear();
    ss.str( string() );
}

void LocalSymbolTable::clear_current_level() {
    for ( auto& it: sym_table ) {
        if ( it.second.empty() ) continue;
        SymTabEntry *entry = ( it.second ).front();
        if ( !entry ) continue;
        else if ( entry->level == current_level ) {
            //Reducing the offset to the min offset in this level;
            offset = offset > entry->offset ? entry->offset : offset;
            it.second.pop_front();
        }
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
	
        symbol->id = (is_arg) ? (FUN_ARG_MASK | symbol_id++) : (LOCAL_SYM_MASK | symbol_id++); 
        
        q.push_front( symbol );
        sym_table.insert( {symbol->name, q} );
        // CSV
        ss << "local," << function_name << "," << id->value << "," << symbol->type.get_name() << "," << symbol->offset << "\n";
    } 
    else {
        deque<SymTabEntry *> &q = it->second;
        if ( q.front() && ( q.front() )->level == current_level ) {
            // Can't insert two symbols with same name at the same level
            error_message( "Redeclaration of symbol " + it->first + " in this scope", id->line_no, id->col_no );
        } 
        else {
            symbol->level = current_level;
			symbol->id = (LOCAL_SYM_MASK | symbol_id++);
            q.push_front( symbol );
            ss << "local," << function_name << "," << id->value << "," << symbol->type.get_name() << "," << symbol->offset << "\n";
        }
    }
}

SymTabEntry *LocalSymbolTable::get_symbol_from_table( string name ) {
    auto it = sym_table.find( name );
    if ( it == sym_table.end() ) return nullptr;
    else {
        if ( it->second.empty() ) return nullptr;
        else return it->second.front();
    }
}

void LocalSymbolTable::add_function( DeclarationSpecifiers *declaration_specifiers, Declarator *declarator, int *error ) {
    sym_table.clear();

    symbol_id = 0;
    offset = 0;
    reqd_size = 0;

    ss.clear();
    ss.str( string() );

    if ( *error == -1 ) return;
    assert( declarator->direct_declarator->type == FUNCTION );

    function_name = declarator->id->value;
    SymTabEntry *gte = global_symbol_table.get_symbol_from_table( function_name );

    if ( gte == nullptr ) {
        *error = -1;
        return;
    }

	return_type = gte->type;
	return_type.is_function = false;
    
    if ( declarator->direct_declarator->params == nullptr ) {
        return;
    }

    vector<ParameterDeclaration *> &param_list = declarator->direct_declarator->params->param_list;

    // Check whether the arguments are of the form ( void )
    if ( param_list.size() == 1 ) {
        auto it = param_list.begin();
        if ( (*it)->declarator == nullptr && (*it)->abstract_declarator == nullptr ) {
            vector<TypeSpecifier *> &v = (*it)->declaration_specifiers->type_specifier;
            if ( v.size() == 1 && ( *v.begin() )->type == VOID ) {
                return;
            }
        }
    }

    // Parameters need to be added at level one, we avoid calling
    // increase_level and clear_from_level to avoid thier side effects
    current_level = 1;

    for ( auto& it: param_list ) {
        int type_index = it->declaration_specifiers->type_index;

        assert( type_index != -1 );
        if ( type_index == -2 ) continue;

        TypeSpecifier *ts = *(it->declaration_specifiers->type_specifier.begin());

        if ( it->declarator == nullptr || it->declarator->id == nullptr ) {
            error_message( "Formal argument requires identifier", ts->line_no, ts->col_no + ts->name.size() );
            continue;
        }

        Type t = it->type;
        if ( t.isVoid() ) {
            error_message( "Invalid type void for argument " + it->declarator->id->value, ts->line_no, ts->col_no );
            continue;
        }

        SymTabEntry *symbol = new SymTabEntry( it->declarator->id->value, it->declarator->id->line_no, it->declarator->id->col_no );
        symbol->type = t;
        add_to_table( symbol, it->declarator->id , true );

    }
    current_level = 0;
    write_to_symtab_file( ss.str() );
    ss.clear();
    ss.str( string() );
}

//########################### GLOBAL SYMBOL TABLE

void GlobalSymbolTable::add_symbol( DeclarationSpecifiers *declaration_specifiers, Declarator *declarator, int *error ) {
    declaration_specifiers->create_type();
    int type_index = declaration_specifiers->type_index;
    int pointer_level = declarator->get_pointer_level();

    assert( type_index != -1 );
    if ( type_index == -2 ) {
        *error = -1;
        return;
    }
    assert( declarator->direct_declarator->type == FUNCTION );

    SymTabEntry *ste = new SymTabEntry( declarator->id->value, declarator->id->line_no, declarator->id->col_no );

    ste->type = Type( type_index, pointer_level, true );
    ste->type.is_function = true;

    DirectDeclarator *dd = declarator->direct_declarator;
    if ( dd->params != nullptr ) {
        ste->type.num_args = dd->params->param_list.size();
        bool fun_args_valid = true;
        for ( auto& j: dd->params->param_list ) {
            if ( j->type.is_invalid() ) {
                fun_args_valid = false;
                break;
            } 
            else ste->type.arg_types.push_back(j->type);
        }

        if ( !fun_args_valid ) {
            delete ste;
            *error = -1;
            return;
        }

    } else {
        ste->type.num_args = 0;
    }

	ste->id = (GLOBAL_SYM_MASK | symbol_id++);
	//MSB 1 -> GLOBAL_SYMBOL 
    add_to_table( ste, true, declarator->id );
    ss << "global," << "-," << declarator->id->value << ",fun:" << ste->type.get_name() << ",0\n";

    write_to_symtab_file( ss.str() );
    ss.clear();
    ss.str( string() );

}

SymTabEntry *GlobalSymbolTable::get_symbol_from_table( string name ) {
    auto it = sym_table.find( name );
    if ( it == sym_table.end() ) return nullptr;
    else return it->second;
}

void GlobalSymbolTable::add_to_table( SymTabEntry *symbol, bool redef, Identifier *id ) {
    auto it = sym_table.find( symbol->name );
    if ( it == sym_table.end() ) {
		symbol->id = (GLOBAL_SYM_MASK | symbol_id++);
		//MSB 1 -> GLOBAL_SYMBOL 
        sym_table.insert( {symbol->name, symbol} );
    } 
    else if ( it->second->type != symbol->type ) {
        error_message( "Conflicting types for " + it->first, id->line_no, id->col_no );
    }
}

Node *add_to_global_symbol_table( Declaration *declaration ) {
    declaration->add_to_symbol_table( global_symbol_table );
    return declaration;
}

void write_to_symtab_file( string s ) { sym_ss << s; }

//########################### SYMBOL TABLE ENTRY

SymTabEntry::SymTabEntry( string _name, unsigned int _line_no, unsigned int _col_no )
    : name( _name ), line_no( _line_no ), col_no( _col_no ), offset(0) {};

//############################## ERROR MESSAGE

void error_message( string str, unsigned int ln, unsigned int col_no ) {
	if ( col_no <= 0 ) {
		error_message(str,ln);
		return;
	}

    if ( ln == ( code.size() + 1 ) || ln == 0) {
        cout << "\nLine: " << line_no << ":" << col_no << " ERROR: " << str << "\n\t" << text.str();
    } 
    else {
        cout << "\nLine: " << ln << ":" << col_no << " ERROR: " << str << "\n\t" << code[ln - 1];
    }
    error_flag = 1;
    printf("\n\t%*s\n", col_no, "^");
}

void error_message( string str, unsigned int ln ) {
    if ( ln == ( code.size() + 1 ) || ln == 0) {
        cout << "\nLine: " << line_no << " ERROR: " << str << "\n\t" << text.str();
    } 
    else {
        cout << "\nLine: " << ln << " ERROR: " << str << "\n\t" << code[ln - 1];
    }
    cout << "\n";
	error_flag = 1;
}

void warning_message( string str, unsigned int ln, unsigned int col_no ) {
	if ( col_no <= 0 ) {
		warning_message(str,ln);
		return;
	}

    if ( ln == ( code.size() + 1 ) || ln == 0) {
        cout << "\nLine: " << line_no << ":" << col_no << " WARNING: " << str << "\n\t" << text.str();
    } 
    else {
        cout << "\nLine: " << ln << ":" << col_no << " WARNING: " << str << "\n\t" << code[ln - 1];
    }
    printf("\n\t%*s\n", col_no, "^");
}

void warning_message( string str, unsigned int ln ) {
    if ( ln == ( code.size() + 1 ) || ln == 0) {
	    cout << "\nLine: " << line_no << " WARNING: " << str << "\n\t" << text.str();
    } else {
	    cout << "\nLine: " << ln << " WARNING: " << str << "\n\t" << code[ln - 1];
    }
    cout << "\n";
}
