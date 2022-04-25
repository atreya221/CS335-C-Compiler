void printi(int x);
void printc(char c);
void exit();

int main() {
	int a[10];
	int i;
	for ( i = 0; i < 10; i ++ ) {
		a[i] = i;
	}
	for ( i = 0; i < 10; i ++ ) {
		printi(a[i]);
		printc('\n');
	}
	exit();
	return 0;
}
