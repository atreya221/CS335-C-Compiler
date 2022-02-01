#include<stdio.h>

//To calculate the fibonacci series
int fib(int n){
    static int cnt = 1;
    cnt++;
    if(n==1) return 1;
    if(n==2) return 1;
    return fib(n-1) + fib(n-2);
}

int main(void){
    int number;
    printf("Fibonacci till: ");  // print fibonacci numbers till a given no
    scanf("%d", &number);
    int i = 1;
    while(i <= number){
        printf("%d ", fib(i));
        i++;
    }
    printf("\n");
    return 0;
}

/*
Line#     Column#   Token               Lexeme                        
4         1         INT                 int
4         5         IDENTIFIER          fib
4         8         (                   (
4         9         INT                 int
4         13        IDENTIFIER          n
4         14        )                   )
4         15        {                   {
5         5         STATIC              static
5         12        INT                 int
5         16        IDENTIFIER          cnt
5         20        =                   =
5         22        CONSTANT            1
5         23        ;                   ;
6         5         IDENTIFIER          cnt
6         8         INC_OP              ++
6         10        ;                   ;
7         5         IF                  if
7         7         (                   (
7         8         IDENTIFIER          n
7         9         EQ_OP               ==
7         11        CONSTANT            1
7         12        )                   )
7         14        RETURN              return
7         21        CONSTANT            1
7         22        ;                   ;
8         5         IF                  if
8         7         (                   (
8         8         IDENTIFIER          n
8         9         EQ_OP               ==
8         11        CONSTANT            2
8         12        )                   )
8         14        RETURN              return
8         21        CONSTANT            1
8         22        ;                   ;
9         5         RETURN              return
9         12        IDENTIFIER          fib
9         15        (                   (
9         16        IDENTIFIER          n
9         17        -                   -
9         18        CONSTANT            1
9         19        )                   )
9         21        +                   +
9         23        IDENTIFIER          fib
9         26        (                   (
9         27        IDENTIFIER          n
9         28        -                   -
9         29        CONSTANT            2
9         30        )                   )
9         31        ;                   ;
10        1         }                   }
12        1         INT                 int
12        5         IDENTIFIER          main
12        9         (                   (
12        10        VOID                void
12        14        )                   )
12        15        {                   {
13        5         INT                 int
13        9         IDENTIFIER          number
13        15        ;                   ;
14        5         IDENTIFIER          printf
14        11        (                   (
14        12        STRING_LITERAL      "Fibonacci till: "
14        30        )                   )
14        31        ;                   ;
15        38        IDENTIFIER          scanf
15        43        (                   (
15        44        STRING_LITERAL      "%d"
15        48        ,                   ,
15        50        &                   &
15        51        IDENTIFIER          number
15        57        )                   )
15        58        ;                   ;
16        5         INT                 int
16        9         IDENTIFIER          i
16        11        =                   =
16        13        CONSTANT            1
16        14        ;                   ;
17        5         WHILE               while
17        10        (                   (
17        11        IDENTIFIER          i
17        13        LE_OP               <=
17        16        IDENTIFIER          number
17        22        )                   )
17        23        {                   {
18        9         IDENTIFIER          printf
18        15        (                   (
18        16        STRING_LITERAL      "%d "
18        21        ,                   ,
18        23        IDENTIFIER          fib
18        26        (                   (
18        27        IDENTIFIER          i
18        28        )                   )
18        29        )                   )
18        30        ;                   ;
19        9         IDENTIFIER          i
19        10        INC_OP              ++
19        12        ;                   ;
20        5         }                   }
21        5         IDENTIFIER          printf
21        11        (                   (
21        12        STRING_LITERAL      "\n"
21        16        )                   )
21        17        ;                   ;
22        5         RETURN              return
22        12        CONSTANT            0
22        13        ;                   ;
23        1         }                   }
*/