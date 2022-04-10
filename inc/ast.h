#ifndef __AST_H__
#define __AST_H__

#include <vector>
#include <string>
#include <fstream>

using namespace std;

template<typename Base, typename T>
inline bool instanceof(const T*) {
   return is_base_of<Base, T>::value;
}

class Node {
	public:
		unsigned long long int id;
		int is_written = 1;
		int line_no;
		int col_no;

		virtual void add_child(Node * child);
		virtual void add_children (Node * child1, Node * child2);
		virtual void add_children (Node * child1, Node * child2, Node * child3);
		virtual void add_children (Node * child1, Node * child2, Node * child3, Node * child4);
		virtual void dotify() = 0;

		unsigned long long int get_id();

	protected:
		Node ();
		Node (int line_no, int col_no);
		virtual ~Node() {}
};


class Terminal : public Node {
	public:
		string name;
		string value;
		void dotify();
		Terminal(const char * string_name, const char * string_value);
		Terminal(const char * string_name, const char * string_value, int line_no, int col_no);
		
};

class Non_Terminal : public Node {
	public:
		string name;
		vector <Node *> children;
		void dotify();
		
		void add_child(Node * child);
		void add_children (Node * child1, Node * child2);
		void add_children (Node * child1, Node * child2, Node * child3);
		void add_children (Node * child1, Node * child2, Node * child3, Node * child4);

		Non_Terminal(const char * string_name);

};

unsigned long long int get_next_node_id();
void file_writer(std::string s);

Node * create_terminal(const char * name, const char * value);
Terminal * create_terminal(const char * name, const char * value, unsigned int line_no, unsigned int col_no);

Node * create_non_term(const char * name);
Node * create_non_term(const char* name, Node* child1);
Node * create_non_term(const char* name, Node* child1, Node* child2);
Node * create_non_term(const char* name, Node* child1, Node* child2, Node* child3);
Node * create_non_term(const char* name, Node* child1, Node* child2, Node* child3, Node* child4);

#endif
