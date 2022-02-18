#include<stdio.h>

int conditional(){
    int x = 5;
    int y = 10;
    int z;
    if(x == 4)
        if(y == 11)
            z = 2;
        else
            z = 10;
    return z;
}


int main(){
    printf("value of z is %d\n", conditional());
    return 0;
}