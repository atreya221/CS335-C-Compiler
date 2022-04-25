void printi(int x);
int scani();
void printc(char ch);
int main()
{
	int x;
	int *p;	
	x = 4;
	p = &x;
	*p = 0;

	printi(x);
	printc('\n');
	return 0;
}