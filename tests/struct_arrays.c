void printi(int );
void printc(char );
void prints(char *);

struct Point{
    int x;
    int y;
};

int main(){
    struct Point points[20];
    int i;
    for(i = 0; i < 20; i++){
        points[i].x = (i+1)*10 + 3*i;
        points[i].y = (i+2)*13 + 2 + i;
    }

    for(i = 0; i < 20; i++){
        prints("Point Number ");
        printi(i+1);
        prints(" is (");
        printi(points[i].x);
        prints(", ");
        printi(points[i].y);
        prints(")\n");
    }

    return 0;
}