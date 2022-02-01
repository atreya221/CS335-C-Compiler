#include<stdio.h>

int main()
{
    int a = 3;
    switch (a)
    {
    case 1:
        printf("a = 1\n");
        break;
    case 2:
        printf("a = 2\n");
        break;
    case 3:
        printf("a = 3\n");
        break;
    case 4:
        printf("a = 4\n");
        break;
    default:
        printf("a = nothing\n");
        break;
    }
    return 0;
}

/*
Line#     Column#   Token               Lexeme                        
3         1         INT                 int
3         5         IDENTIFIER          main
3         9         (                   (
3         10        )                   )
4         1         {                   {
5         5         INT                 int
5         9         IDENTIFIER          a
5         11        =                   =
5         13        CONSTANT            3
5         14        ;                   ;
6         5         SWITCH              switch
6         12        (                   (
6         13        IDENTIFIER          a
6         14        )                   )
7         5         {                   {
8         5         CASE                case
8         10        CONSTANT            1
8         11        :                   :
9         9         IDENTIFIER          printf
9         15        (                   (
9         16        STRING_LITERAL      "a = 1\n"
9         25        )                   )
9         26        ;                   ;
10        9         BREAK               break
10        14        ;                   ;
11        5         CASE                case
11        10        CONSTANT            2
11        11        :                   :
12        9         IDENTIFIER          printf
12        15        (                   (
12        16        STRING_LITERAL      "a = 2\n"
12        25        )                   )
12        26        ;                   ;
13        9         BREAK               break
13        14        ;                   ;
14        5         CASE                case
14        10        CONSTANT            3
14        11        :                   :
15        9         IDENTIFIER          printf
15        15        (                   (
15        16        STRING_LITERAL      "a = 3\n"
15        25        )                   )
15        26        ;                   ;
16        9         BREAK               break
16        14        ;                   ;
17        5         CASE                case
17        10        CONSTANT            4
17        11        :                   :
18        9         IDENTIFIER          printf
18        15        (                   (
18        16        STRING_LITERAL      "a = 4\n"
18        25        )                   )
18        26        ;                   ;
19        9         BREAK               break
19        14        ;                   ;
20        5         DEFAULT             default
20        12        :                   :
21        9         IDENTIFIER          printf
21        15        (                   (
21        16        STRING_LITERAL      "a = nothing\n"
21        31        )                   )
21        32        ;                   ;
22        9         BREAK               break
22        14        ;                   ;
23        5         }                   }
24        5         RETURN              return
24        12        CONSTANT            0
24        13        ;                   ;
25        1         }                   }
*/