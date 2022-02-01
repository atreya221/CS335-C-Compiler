#include <stdio.h>
#include <stdlib.h>

int main()
{
    int num;
    FILE *fptr;

    fptr = fopen("./program.txt","w");

    if(fptr == NULL)
    {
        printf("Error!");   
        exit(1);             
    }

    printf("Enter num: ");
    scanf("%d",&num);

    fprintf(fptr,"%d",num);
    fclose(fptr);

    return 0;
}


/*
Line#     Column#   Token               Lexeme                        
4         1         INT                 int
4         5         IDENTIFIER          main
4         9         (                   (
4         10        )                   )
5         1         {                   {
6         5         INT                 int
6         9         IDENTIFIER          num
6         12        ;                   ;
7         5         FILE                FILE
7         10        *                   *
7         11        IDENTIFIER          fptr
7         15        ;                   ;
9         5         IDENTIFIER          fptr
9         10        =                   =
9         12        IDENTIFIER          fopen
9         17        (                   (
9         18        STRING_LITERAL      "./program.txt"
9         33        ,                   ,
9         34        STRING_LITERAL      "w"
9         37        )                   )
9         38        ;                   ;
11        5         IF                  if
11        7         (                   (
11        8         IDENTIFIER          fptr
11        13        EQ_OP               ==
11        16        IDENTIFIER          NULL
11        20        )                   )
12        5         {                   {
13        9         IDENTIFIER          printf
13        15        (                   (
13        16        STRING_LITERAL      "Error!"
13        24        )                   )
13        25        ;                   ;
14        9         IDENTIFIER          exit
14        13        (                   (
14        14        CONSTANT            1
14        15        )                   )
14        16        ;                   ;
15        5         }                   }
17        5         IDENTIFIER          printf
17        11        (                   (
17        12        STRING_LITERAL      "Enter num: "
17        25        )                   )
17        26        ;                   ;
18        5         IDENTIFIER          scanf
18        10        (                   (
18        11        STRING_LITERAL      "%d"
18        15        ,                   ,
18        16        &                   &
18        17        IDENTIFIER          num
18        20        )                   )
18        21        ;                   ;
20        5         IDENTIFIER          fprintf
20        12        (                   (
20        13        IDENTIFIER          fptr
20        17        ,                   ,
20        18        STRING_LITERAL      "%d"
20        22        ,                   ,
20        23        IDENTIFIER          num
20        26        )                   )
20        27        ;                   ;
21        5         IDENTIFIER          fclose
21        11        (                   (
21        12        IDENTIFIER          fptr
21        16        )                   )
21        17        ;                   ;
23        5         RETURN              return
23        12        CONSTANT            0
23        13        ;                   ;
24        1         }                   }
*/