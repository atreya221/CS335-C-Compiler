void printi(int c);
void printc(char c);
int scani();
int main(){
    int a,b,c;
    a = 14*3+5+(4+(8*12/73))-23*76+48;
    printi(a);
    printc('\n');
    b = a*2;
    printi(b);
    printc('\n');

    b= a/16;
    printi(b);
    printc('\n');

    b=a*0;
    printi(b);
    printc('\n');

    b=a+0;
    printi(b);
    printc('\n');
    
    return 0;
}