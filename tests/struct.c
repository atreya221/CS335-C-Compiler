void printi(int x);
void printc(char c);
void prints(char* str);
struct point{
    int x,y;
};
int main(){
    int i,a,b,c;
    int *ptr1,*ptr2,*ptr3;
    int arr[10];
    struct point p1,p2;
    struct point* p3,*p4;

    prints("Sizeof struct: ");
	printi(sizeof(struct point));
    printc('\n');

    a=4;
    b=3;
    c=7;
    p1.x=2,p1.y=3;
    p2.x=3,p2.y=4;
    ptr1=&a;
    ptr2=&b;


    *ptr1 = 10;
    *ptr2 += *ptr1;
    p3=&p1;
    p4=&p2;
    (*p3).x=5,(*p3).y=6;
    p4->x=7;p4->y=p1.y;
    printi(a);
    printc('\n');
    printi(p1.x);
    printc('\n');
    printi(p1.y);

    printc('\n');
    printi(p2.x);
	printc('\n');
    


    return 0;
}