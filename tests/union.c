void printi(int x);
int scani();
char scanc();
void exit();
void printc(char c);
void prints(char * c);

union data { char a; int b; } u;


int main(){

	prints("Sizeof union: ");
	printi(sizeof(u));
	printc('\n');
	u.b = 0;
	prints("Enter a character: ");
	u.a = scanc();
	printi(u.b);
	printc('\n');
	prints("Enter an integer : ");
	u.b = scani();
	printc(u.a);
	printc('\n');
	
    return 0;
}

