void printi( int x);
void printc( char x);
int scani();
void prints(char *str);
int main(){
    int a,b,c,x;
    int *ptra,*ptrb,*ptrc;
    int **pptra,**pptrb,**pptrc,***ppptrb;
    a=71;
    b=57,c=b*a+34;

    ptra=&a,ptrb=&b,ptrc=&c;
    pptra=&ptra,pptrb=&ptrb;
    ppptrb=&pptrb;
    *(*pptra)=23; 
    ***ppptrb= **pptra+4; 

    prints("pptra and a are: \n");
    printi(**pptra);
    printc('\n');
    printi(a);
    printc('\n');

    prints("ppptrb and b are: \n");
    printi(***ppptrb);
    printc('\n');
    printi(b);

    return 0;
}