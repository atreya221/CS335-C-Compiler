void printi(int x);
void printc(char c);
void prints(char*);
int scani();
int main(){  
    int i;
    prints("Enter a number to travel: ");
    i = scani();

    if (i<10)
        if(i<5)
            if(i==0)
                prints(" == 0");
            else
                prints("< 5 and >= 0");
        else
            prints(">= 5 and < 10");
    else{
        if(i<20)
            prints(">= 10 and < 20");
        else{
            if(i<30)
                prints(">= 20 and < 30");
            else
                prints("> 30");
        }
    }
    printc('\n');
    return 0;
}