void printi(int c);
void printc(char c);
void prints(char *str);
int scani();

int func(){
    prints("Short Circuit Failed");
    return 1;
}
int main(){
    int a,b,c,d;
    a=2,b=5,c=7;
    if( a==3 && func() ){
        b=3;
    }
    printi(a);
    printc(' ');
    printi(b);
    printc(' ');
    printi(c);
    printc('\n');
    if( a==2 || func() ){
        b=6;
    }
    printi(a);
    printc(' ');
    printi(b);
    printc(' ');
    printi(c);
    printc('\n');
}