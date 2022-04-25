void printi(int c);
void printc(char c);
void prints(char * c);
int scani();
struct point{
    int x,y;
};
void foo(){
    int a,b;
    return;
}
int main(){
    int i,j,k,l,*ptr,**ptr2;
    const int a;
    struct point p;
    void *c;
    int arr[5];
    char d;
    float n, m;
    n = 3.5;
    m = 2.5;
    
    a++;
    p.x=3,p.y=5;
    p--;
    ++a;
    ++p;

    // function pointer not supported
    c=(&foo);

    // dereferencing int or an aaray
    *a=3;
    *arr=2;
    // unary + and - 
    j=i+(-ptr);

    //float operation
    m = n + m;

    // multiplicative
    k = (ptr * a)+(ptr/a);
    //relation
    if(a>ptr){
        j=10;
    }
    //and
    j=a&ptr;

    //assignment =
    ptr2=ptr;(//Warning)
    p=ptr;

    //*=
    ptr*=7;

    return 0;
}

