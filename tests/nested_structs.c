void printi(int c);
void printc(char c);
int scani();
struct point_2d{
    int x,y;
};
struct point_3d{
    struct point_2d x;
    int z;
};
int main(){
    int a,b,c;
    struct point_2d f,g;
    struct point_3d h,i;
    f.x=4,f.y=3;
    h.x.x=7;
    h.x.y=31;
    h.z=5;
    printi(h.x.x);
    printc('\n');
    printi(h.x.y);
    printc('\n');
    printi(h.z);
    printc('\n');
}