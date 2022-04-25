void printi( int x);
void printc( char x);
void prints(char * );
void scans( char *, int );
int scani();
void exit( int );


void * sbrk(int);


struct node {
	int key;
	struct node * left;
	struct node * right;
};

void add_to_btree(struct node * root, int key ) {
	if ( key < root->key ){
		struct node * new;
		if (root->left != (struct node *)NULL){
			add_to_btree(root->left,key);
			return;
		}
		new = (struct node * )sbrk(sizeof(struct node));
		if (new == (struct node *) NULL ){
			prints("sbrk failed\n");
			exit(0);
		}
		new->key = key;
		new->left = (struct node *)NULL;
		new->right = (struct node *)NULL;
		root->left = new;
	} else {
		struct node * new;
		if (root->right != (struct node *)NULL){
			add_to_btree(root->right,key);
			return;
		}
		new = (struct node * )sbrk(sizeof(struct node));
		if (new == (struct node *) NULL ){
			prints("sbrk failed\n");
			exit(0);
		}
		new->key = key;
		new->left = (struct node *)NULL;
		new->right = (struct node *)NULL;
		root->right = new;
		return;
	}
}


int main(){
	struct node * root;
	int i,n,k;
	prints("How many nodes to create?\n");
	n = scani();
	if ( n == 0 ) {
		return 0;
	}
	root = (struct node * )sbrk(sizeof(struct node));
	if (root == (struct node *) NULL ){
		prints("sbrk failed\n");
		exit(0);
	}
	prints("Enter values to insert\n");
	k = scani();
	root->key = k;
	root->left = (struct node *) NULL;
	root->right = (struct node *) NULL;
	for(i = 0; i < n-1; i++ ) {
		k = scani();
		add_to_btree(root,k);
	}
	printc('\n');
	return 0;
}


