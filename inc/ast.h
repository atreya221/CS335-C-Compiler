#ifndef __AST_H__
#define __AST_H__

#include <string>
#include <vector>
#include <fstream>

using namespace std;

template<typename Base, typename T>
inline bool instanceof(const T*) {
   return is_base_of<Base, T>::value;
}

class Node {
	public:
		unsigned long long int id;
		int is_printed = 1;
		unsigned int line_no;
		unsigned int col_no;

		virtual void dotify() = 0;
		virtual void add_child(Node * node);
		virtual void add_children (Node * node1, Node * node2);
		virtual void add_children (Node * node1, Node * node2, Node * node3);
		virtual void add_children (Node * node1, Node * node2, Node * node3, Node * node4);
		unsigned long long int get_id();
	protected:
		Node ();
		Node (unsigned int line_no, unsigned int col_no );
		virtual ~Node() {}
};


class Terminal : public Node {
	public:
		string name;
		string value;
		Terminal(const char * name_, const char * value_);
		Terminal(const char * name_, const char * value_,  unsigned int _line_no, unsigned int _col_no );
		void dotify();
		
};

class Non_Terminal : public Node {
	public:
		string name;
		vector <Node *> children;
		

		Non_Terminal(const char * name_);
		void add_child(Node * node);
		void add_children (Node * node1, Node * node2);
		void add_children (Node * node1, Node * node2, Node * node3);
		void add_children (Node * node1, Node * node2, Node * node3, Node * node4);
		void dotify();

};

unsigned long long int get_next_node_id();
Node * create_terminal(const char * name, const char * value);
Terminal * create_terminal(const char * name, const char * value, unsigned int line_no, unsigned int col_no);
Node * create_non_term(const char * name);
Node * create_non_term(const char* name, Node* node1, Node* node2, Node* node3, Node* node4, Node* node5, Node* node6, Node* node7);
Node * create_non_term(const char* name, Node* node1, Node* node2, Node* node3, Node* node4, Node* node5, Node* node6);
Node * create_non_term(const char* name, Node* node1, Node* node2, Node* node3, Node* node4, Node* node5);
Node * create_non_term(const char* name, Node* node1, Node* node2, Node* node3, Node* node4);
Node * create_non_term(const char* name, Node* node1, Node* node2, Node* node3);
Node * create_non_term(const char* name, Node* node1, Node* node2);
Node * create_non_term(const char* name, Node* node1);

void file_writer(string s);

#endif
