void printi(int x);
void printc(char c);

int main() {
	int a[10][10];
	int i,j;
	for ( i = 0; i < 10; i++ ) {
		for ( j = 0; j < 10; j++ ) {
			a[i][j] = i + j;
		}
	}
	for ( i = 0; i < 10; i++ ) {
		for ( j = 0; j < 10; j++ ) {
			printi(a[i][j]);
			printc(' ');
		}
		printc('\n');
	}
	return 0;
}
