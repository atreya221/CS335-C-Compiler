void printi(int c);
void printc(char c);
void prints(char * );
int scani();
int pow(int b, int e);
int fpow(int b, int e);
int iseven(int n);
int gcd(int a, int b);
int max(int a, int b);
int min(int a, int b);


int main(){
    int a,b;
    prints("Enter first number: ");
    a=scani();
    prints("Enter second number: ");
    b=scani();
    prints("power of a and b is: ");
    printi(pow(a,b));
    prints("\nmax of a and b is: ");
    printi(max(a,b));
    prints("\nmin of a and b is: ");
    printi(min(a,b));
    prints("\ngcd of a and b is: ");
    printi(gcd(a,b));
    prints("\n");
}
