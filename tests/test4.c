#include<stdio.h>

void main()
{
    unsigned char a = 5, b = 9; 
    int c = 0x125;
    int e = 1e10;
    printf("a+b = %d\n", a+b);
    printf("a-b = %d\n", a-b);
    printf("a*b = %d\n", a*b);
    printf("a/b = %d\n", a/b);
    printf("a%b = %d\n", a%b);
    printf("a^b = %d\n", a^b);

    printf("c = %d\n", c);  
    printf("e = %d\n", e);  
    
    printf("a<<1 = %d\n", a<<1);
    printf("b<<1 = %d\n", b<<1);
    printf("a>>1 = %d\n", a>>1);
    printf("b>>1 = %d\n", b>>1);  
}


/*
Line#     Column#   Token               Lexeme                        
3         1         VOID                void
3         6         IDENTIFIER          main
3         10        (                   (
3         11        )                   )
4         1         {                   {
5         5         UNSIGNED            unsigned
5         14        CHAR                char
5         19        IDENTIFIER          a
5         21        =                   =
5         23        CONSTANT            5
5         24        ,                   ,
5         26        IDENTIFIER          b
5         28        =                   =
5         30        CONSTANT            9
5         31        ;                   ;
6         5         INT                 int
6         9         IDENTIFIER          c
6         11        =                   =
6         13        CONSTANT            0x125
6         18        ;                   ;
7         5         INT                 int
7         9         IDENTIFIER          e
7         11        =                   =
7         13        CONSTANT            1e10
7         17        ;                   ;
8         5         IDENTIFIER          printf
8         11        (                   (
8         12        STRING_LITERAL      "a+b = %d\n"
8         24        ,                   ,
8         26        IDENTIFIER          a
8         27        +                   +
8         28        IDENTIFIER          b
8         29        )                   )
8         30        ;                   ;
9         5         IDENTIFIER          printf
9         11        (                   (
9         12        STRING_LITERAL      "a-b = %d\n"
9         24        ,                   ,
9         26        IDENTIFIER          a
9         27        -                   -
9         28        IDENTIFIER          b
9         29        )                   )
9         30        ;                   ;
10        5         IDENTIFIER          printf
10        11        (                   (
10        12        STRING_LITERAL      "a*b = %d\n"
10        24        ,                   ,
10        26        IDENTIFIER          a
10        27        *                   *
10        28        IDENTIFIER          b
10        29        )                   )
10        30        ;                   ;
11        5         IDENTIFIER          printf
11        11        (                   (
11        12        STRING_LITERAL      "a/b = %d\n"
11        24        ,                   ,
11        26        IDENTIFIER          a
11        27        /                   /
11        28        IDENTIFIER          b
11        29        )                   )
11        30        ;                   ;
12        5         IDENTIFIER          printf
12        11        (                   (
12        12        STRING_LITERAL      "a%b = %d\n"
12        24        ,                   ,
12        26        IDENTIFIER          a
12        27        %                   %
12        28        IDENTIFIER          b
12        29        )                   )
12        30        ;                   ;
13        5         IDENTIFIER          printf
13        11        (                   (
13        12        STRING_LITERAL      "a^b = %d\n"
13        24        ,                   ,
13        26        IDENTIFIER          a
13        27        ^                   ^
13        28        IDENTIFIER          b
13        29        )                   )
13        30        ;                   ;
15        5         IDENTIFIER          printf
15        11        (                   (
15        12        STRING_LITERAL      "c = %d\n"
15        22        ,                   ,
15        24        IDENTIFIER          c
15        25        )                   )
15        26        ;                   ;
16        5         IDENTIFIER          printf
16        11        (                   (
16        12        STRING_LITERAL      "e = %d\n"
16        22        ,                   ,
16        24        IDENTIFIER          e
16        25        )                   )
16        26        ;                   ;
18        5         IDENTIFIER          printf
18        11        (                   (
18        12        STRING_LITERAL      "a<<1 = %d\n"
18        25        ,                   ,
18        27        IDENTIFIER          a
18        28        LEFT_OP             <<
18        30        CONSTANT            1
18        31        )                   )
18        32        ;                   ;
19        5         IDENTIFIER          printf
19        11        (                   (
19        12        STRING_LITERAL      "b<<1 = %d\n"
19        25        ,                   ,
19        27        IDENTIFIER          b
19        28        LEFT_OP             <<
19        30        CONSTANT            1
19        31        )                   )
19        32        ;                   ;
20        5         IDENTIFIER          printf
20        11        (                   (
20        12        STRING_LITERAL      "a>>1 = %d\n"
20        25        ,                   ,
20        27        IDENTIFIER          a
20        28        RIGHT_OP            >>
20        30        CONSTANT            1
20        31        )                   )
20        32        ;                   ;
21        5         IDENTIFIER          printf
21        11        (                   (
21        12        STRING_LITERAL      "b>>1 = %d\n"
21        25        ,                   ,
21        27        IDENTIFIER          b
21        28        RIGHT_OP            >>
21        30        CONSTANT            1
21        31        )                   )
21        32        ;                   ;
22        1         }                   }
*/