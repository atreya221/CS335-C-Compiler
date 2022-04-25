void printi(int c);
void printc(char c);
void prints(char *str);
int scani();

int main(){
    int t,a;
    int *x;
    x = &a;
    t=3,a=1;
    while( a < 7 && a > 9 || t != 4 ){
        a+=2;
        a*=3;
        t++;
        prints("Inside while\n");
    }
    t++;
    printi(a);
    printc('\n');
    a=7;
    do {
        a--;
        prints("Inside do_while\n");
    }
    while(a<5);
    
    
    t=0;
    for(a=1;a<5;a++){
        t+=a;   
    }
    printi(t);
    printc('\n');
    printi(a);
    printc('\n');
    while(a--){
        for(t=a;t>0;t--){
            prints("Inner_loop\n");
        }
        prints("Outer_loop\n");
    }


    return 0;
}