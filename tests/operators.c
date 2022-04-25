void printi(int c);
void printc(char c);
void exit();
int main(){
    int a,b,c,d;
    a = 2;
    b = -3;
    c = 5;
    d = 7;
    d=a+b; 
    printi(d);
    printc('\n'); 

    printi(c); 
    printc('\n');
    c = (-a) ;
    printi(c);
    printc('\n');


    c = c^b;
    printi(c);
    printc('\n');
    c = c&a | (c|b);
    printi(c);
    printc('\n');
    c = 2>>1;
    printi(c);
    printc('\n');
    c = 2<<1;
    printi( c);
    printc('\n');
    c = c>>b;
    printi(c);
    printc('\n');




    c = (b >= a);
    printi(c);
    printc('\n');
    c = (b <= a);
    printi(c);
    printc('\n');
    c = (a < b);
    printi(c);
    printc('\n');
    c = (a > b);
    printi(c);
    printc('\n');
    printc('\n');
    c += 10*(-1);
    printi(c);
    printc('\n');
    c *= 15;
    printi(c);
    printc('\n');
    c /= 3;
    printi(c);
    printc('\n');
    c <<= 1;
    printi(c);
    printc('\n');
    c >>= 2;
    printi(c);
    printc('\n');
    c ^= 1000;
    printi(c);
    printc('\n');
    c &= 10;
    printi(c);
    printc('\n');
    c |= 15;
    printi(c);
    printc('\n');
    
   return 0;
} 
    
