void printi(int c);
void printc(char c);
void prints(char *str);
int scani();
int main(){
    int x;
    prints("Enter a number: ");
    x=scani();
    switch(x){
        case 1 : prints("x==1\n");
                break;
        case 2: prints("x==2\n");
                break;
        case 3: prints("x==3\n");
                break;
        default :
                prints("x>3\n");
    }

    x=scani();
    switch(x){
        case 1 : prints("case1\n");
                break;
        default :
                prints("default\n");

        case 2: prints("case2\n");
                
        case 3: prints("case3\n");
                break;
       
    }

    return 0 ;

}