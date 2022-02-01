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
6         4         INT                 int
6         8         IDENTIFIER          num
6         11        ;                   ;
7         4         FILE                FILE
7         9         *                   *
7         10        IDENTIFIER          fptr
7         14        ;                   ;
10        7         IDENTIFIER          fptr
10        12        =                   =
10        14        IDENTIFIER          fopen
10        19        (                   (
10        20        STRING_LITERAL      "./program.txt"
10        37        ,                   ,
10        38        STRING_LITERAL      "w"
10        41        )                   )
10        42        ;                   ;
12        4         IF                  if
12        6         (                   (
12        7         IDENTIFIER          fptr
12        12        EQ_OP               ==
12        15        IDENTIFIER          NULL
12        19        )                   )
13        4         {                   {
14        7         IDENTIFIER          printf
14        13        (                   (
14        14        STRING_LITERAL      "Error!"
14        22        )                   )
14        23        ;                   ;
15        7         IDENTIFIER          exit
15        11        (                   (
15        12        CONSTANT            1
15        13        )                   )
15        14        ;                   ;
16        4         }                   }
18        4         IDENTIFIER          printf
18        10        (                   (
18        11        STRING_LITERAL      "Enter num: "
18        24        )                   )
18        25        ;                   ;
19        4         IDENTIFIER          scanf
19        9         (                   (
19        10        STRING_LITERAL      "%d"
19        14        ,                   ,
19        15        &                   &
19        16        IDENTIFIER          num
19        19        )                   )
19        20        ;                   ;
21        4         IDENTIFIER          fprintf
21        11        (                   (
21        12        IDENTIFIER          fptr
21        16        ,                   ,
21        17        STRING_LITERAL      "%d"
21        21        ,                   ,
21        22        IDENTIFIER          num
21        25        )                   )
21        26        ;                   ;
22        4         IDENTIFIER          fclose
22        10        (                   (
22        11        IDENTIFIER          fptr
22        15        )                   )
22        16        ;                   ;
24        4         RETURN              return
24        11        CONSTANT            0
24        12        ;                   ;
25        1         }                   }
*/