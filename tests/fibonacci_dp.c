void printi(int i);
void printc(char ch);
void prints(char *str);
int scani();

int fibs[1001];
int x;

int fib(int n) {
	int result;
	if(fibs[n] != -1){
		result = fibs[n];
		return result;
	}
	if ( n == 1 ) {
		fibs[1] = 0;
		return (int) 0;
	} else if ( n == 2 ) {
		fibs[2] = 1;
		return (int) 1;
	}
	result = fib(n-1) + fib(n-2);
	fibs[n] = result;
	return result;
}

int main() {
	int i = 0;
	int y;
	prints("Enter a number: ");
	x = scani();
	for(i=0;i<1001;i++){
		int j;
		fibs[i] = -1;

	}
	prints("Fibonacci series: \n");
	y = fib(x);	
	printi(y);
	printc('\n');

	for(i = 0; i <= x; i++){
		printi(fibs[i]);
		printc(' ');
	}

	printc('\n');

	return 0;
}
