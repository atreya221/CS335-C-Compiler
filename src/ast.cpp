#include<iostream>
#include<string>
#include<assert.h>
#include<sstream>

#include "ast.h"

using namespace std;

static unsigned long long int node_id_count = 0;

unsigned long long int get_next_node_id() {
	return node_id_count++;
}

Node::Node() : node_id(get_next_node_id()) {};

Terminal::Terminal(const char * string_name, const char * string_value) {
	name = string(string_name);
	if (string_value) {
		value = string(string_value);
	}
}

Non_Terminal::Non_Terminal(const char * string_name) {
	name = string(string_name);
}


void Node::add_child (Node * child) {}
void Node::add_children (Node * child1, Node * child2) {}
void Node::add_children (Node * child1, Node * child2, Node * child3) {}
void Node::add_children (Node * child1, Node * child2, Node * child3, Node * child4) {}

void Non_Terminal::add_child (Node * child) {
	if( child )	children.push_back(child);
}

void Non_Terminal::add_children (Node * child1, Node * child2) {
	if( child1 ) children.push_back(child1);
	if( child2 ) children.push_back(child2);
}

void Non_Terminal::add_children (Node * child1, Node * child2, Node * child3) {
	if( child1 ) children.push_back(child1);
	if( child2 ) children.push_back(child2);
	if( child3 ) children.push_back(child3);
}

void Non_Terminal::add_children (Node * child1, Node * child2, Node * child3, Node * child4) {
	if( child1 ) children.push_back(child1);
	if( child2 ) children.push_back(child2);
	if( child3 ) children.push_back(child3);
	if( child4 ) children.push_back(child4);
}

Node * create_terminal(const char* name, const char * value) {
	Terminal * new_terminal_node = new Terminal(name,value);
	return new_terminal_node;
}


Node * create_non_term(const char* name) {
	Non_Terminal * new_node = new Non_Terminal(name);
	return new_node;
}

Node * create_non_term(const char* name, Node* child1) {
	Non_Terminal * new_node = new Non_Terminal(name);
	new_node->add_child(child1);
	if(new_node->children.empty() == false){
		return new_node;
	}
	else {
		delete new_node;
		return NULL;
	}
}

Node * create_non_term(const char* name, Node* child1, Node* child2) {
	Non_Terminal * new_node = new Non_Terminal(name);
	new_node->add_children(child1, child2);
	if(new_node->children.empty() == false ){
		return new_node;
	}
	else {
		delete new_node;
		return NULL;
	}
}

Node * create_non_term(const char* name, Node* child1, Node* child2, Node* child3) {
	Non_Terminal * new_node = new Non_Terminal(name);
	new_node->add_children(child1, child2, child3);
	if(new_node->children.empty() == false ){
		return new_node;
	}
	else {
		delete new_node;
		return NULL;
	}
}

Node * create_non_term(const char* name, Node* child1, Node* child2, Node* child3, Node* child4) {
	Non_Terminal * new_node = new Non_Terminal(name);
	new_node->add_children(child1, child2, child3, child4);

	if(new_node->children.empty() == false){
		return new_node;
	}
	else {
		delete new_node;
		return NULL;
	}
}

void Terminal:: dotify () {
	if(is_written == 1){
		is_written = 0;
		stringstream ss;
		ss << "\t" << node_id << " [label=\"" << name << "\"];\n";
		file_writer(ss.str());
	}
}
void Non_Terminal:: dotify () {
	if(is_written == 1){
		is_written = 0;
		stringstream ss;
		ss << "\t" << node_id << " [label=\"" << name << "\"];\n";
		auto it = children.begin();
		while(it != children.end()){
			ss << "\t" << node_id << " -> " << (*it)->node_id << ";\n";
			it++;
		}

		file_writer(ss.str());

		it = children.begin();
		while(it != children.end()){	
			(*it)->dotify();
			it++;
		}
	}
}






