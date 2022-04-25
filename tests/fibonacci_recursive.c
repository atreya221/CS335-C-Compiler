void printi(int i);
void printc(char ch);
void prints(char* str);
int scani();

int fib(int n) {
	if ( n == 1 ) {
		return 0;
	} else if ( n == 2 ) {
		return 1;
	}
	return fib(n-1) + fib(n-2);
}

int main() {
	int x;
	int y;
	prints("Enter a number: ");
	x = scani();

	y = fib(x);
	prints("Fibonacci number: ");	
	printi(y);
	printc('\n');
	return 0;
}
