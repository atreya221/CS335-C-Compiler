#include <stdio.h>

//struct of the student
struct student {
    char id[15];
    char firstn[64];
    char lastn[64];
    float score;
};

int main(void) {
    struct student std[3];
    struct student *ptr = NULL;
    
    int i;
    ptr = std;
    printf("size of student : %d\n", sizeof(struct student));
    
    for (i = 0; i < 3; i++) {
        printf("Enter detail of student #%d\n", (i + 1));
        printf("Enter ID: ");
        scanf("%s", ptr->id);
        printf("Enter first name: ");
        scanf("%s", ptr->firstn);
        printf("Enter last name: ");
        scanf("%s", ptr->lastn);
        printf("Enter Points: ");
        scanf("%f", &ptr->score);
        
        ptr++;
    }
    
    ptr = std + 2;
    
    for (i = 2; i >= 0; i--) {
        printf("\nDetail of student #%d\n", (i + 1));
        
        printf("\nResult via std\n");
        printf("ID: %s\n", std[i].id);
        printf("First Name: %s\n", std[i].firstn);
        printf("Last Name: %s\n", std[i].lastn);
        printf("Points: %f\n", std[i].score);

        printf("\nResult via ptr\n");
        printf("ID: %s\n", ptr->id);
        printf("First Name: %s\n", ptr->firstn);
        printf("Last Name: %s\n", ptr->lastn);
        printf("Points: %f\n", ptr->score);
        
        ptr--;
    }
    
    return 0;
}

/*
Line#     Column#   Token               Lexeme                        
4         1         STRUCT              struct
4         8         IDENTIFIER          student
4         16        {                   {
5         5         CHAR                char
5         10        IDENTIFIER          id
5         12        [                   [
5         13        CONSTANT            15
5         15        ]                   ]
5         16        ;                   ;
6         5         CHAR                char
6         10        IDENTIFIER          firstn
6         16        [                   [
6         17        CONSTANT            64
6         19        ]                   ]
6         20        ;                   ;
7         5         CHAR                char
7         10        IDENTIFIER          lastn
7         15        [                   [
7         16        CONSTANT            64
7         18        ]                   ]
7         19        ;                   ;
8         5         FLOAT               float
8         11        IDENTIFIER          score
8         16        ;                   ;
9         1         }                   }
9         2         ;                   ;
11        1         INT                 int
11        5         IDENTIFIER          main
11        9         (                   (
11        10        VOID                void
11        14        )                   )
11        16        {                   {
12        5         STRUCT              struct
12        12        IDENTIFIER          student
12        20        IDENTIFIER          std
12        23        [                   [
12        24        CONSTANT            3
12        25        ]                   ]
12        26        ;                   ;
13        5         STRUCT              struct
13        12        IDENTIFIER          student
13        20        *                   *
13        21        IDENTIFIER          ptr
13        25        =                   =
13        27        IDENTIFIER          NULL
13        31        ;                   ;
15        5         INT                 int
15        9         IDENTIFIER          i
15        10        ;                   ;
16        5         IDENTIFIER          ptr
16        9         =                   =
16        11        IDENTIFIER          std
16        14        ;                   ;
17        5         IDENTIFIER          printf
17        11        (                   (
17        12        STRING_LITERAL      "size of student : %d\n"
17        36        ,                   ,
17        38        SIZEOF              sizeof
17        44        (                   (
17        45        STRUCT              struct
17        52        IDENTIFIER          student
17        59        )                   )
17        60        )                   )
17        61        ;                   ;
19        5         FOR                 for
19        9         (                   (
19        10        IDENTIFIER          i
19        12        =                   =
19        14        CONSTANT            0
19        15        ;                   ;
19        17        IDENTIFIER          i
19        19        <                   <
19        21        CONSTANT            3
19        22        ;                   ;
19        24        IDENTIFIER          i
19        25        INC_OP              ++
19        27        )                   )
19        29        {                   {
20        9         IDENTIFIER          printf
20        15        (                   (
20        16        STRING_LITERAL      "Enter detail of student #%d\n"
20        47        ,                   ,
20        49        (                   (
20        50        IDENTIFIER          i
20        52        +                   +
20        54        CONSTANT            1
20        55        )                   )
20        56        )                   )
20        57        ;                   ;
21        9         IDENTIFIER          printf
21        15        (                   (
21        16        STRING_LITERAL      "Enter ID: "
21        28        )                   )
21        29        ;                   ;
22        9         IDENTIFIER          scanf
22        14        (                   (
22        15        STRING_LITERAL      "%s"
22        19        ,                   ,
22        21        IDENTIFIER          ptr
22        24        PTR_OP              ->
22        26        IDENTIFIER          id
22        28        )                   )
22        29        ;                   ;
23        9         IDENTIFIER          printf
23        15        (                   (
23        16        STRING_LITERAL      "Enter first name: "
23        36        )                   )
23        37        ;                   ;
24        9         IDENTIFIER          scanf
24        14        (                   (
24        15        STRING_LITERAL      "%s"
24        19        ,                   ,
24        21        IDENTIFIER          ptr
24        24        PTR_OP              ->
24        26        IDENTIFIER          firstn
24        32        )                   )
24        33        ;                   ;
25        9         IDENTIFIER          printf
25        15        (                   (
25        16        STRING_LITERAL      "Enter last name: "
25        35        )                   )
25        36        ;                   ;
26        9         IDENTIFIER          scanf
26        14        (                   (
26        15        STRING_LITERAL      "%s"
26        19        ,                   ,
26        21        IDENTIFIER          ptr
26        24        PTR_OP              ->
26        26        IDENTIFIER          lastn
26        31        )                   )
26        32        ;                   ;
27        9         IDENTIFIER          printf
27        15        (                   (
27        16        STRING_LITERAL      "Enter Points: "
27        32        )                   )
27        33        ;                   ;
28        9         IDENTIFIER          scanf
28        14        (                   (
28        15        STRING_LITERAL      "%f"
28        19        ,                   ,
28        21        &                   &
28        22        IDENTIFIER          ptr
28        25        PTR_OP              ->
28        27        IDENTIFIER          score
28        32        )                   )
28        33        ;                   ;
30        9         IDENTIFIER          ptr
30        12        INC_OP              ++
30        14        ;                   ;
31        5         }                   }
33        5         IDENTIFIER          ptr
33        9         =                   =
33        11        IDENTIFIER          std
33        15        +                   +
33        17        CONSTANT            2
33        18        ;                   ;
35        5         FOR                 for
35        9         (                   (
35        10        IDENTIFIER          i
35        12        =                   =
35        14        CONSTANT            2
35        15        ;                   ;
35        17        IDENTIFIER          i
35        19        GE_OP               >=
35        22        CONSTANT            0
35        23        ;                   ;
35        25        IDENTIFIER          i
35        26        DEC_OP              --
35        28        )                   )
35        30        {                   {
36        9         IDENTIFIER          printf
36        15        (                   (
36        16        STRING_LITERAL      "\nDetail of student #%d\n"
36        43        ,                   ,
36        45        (                   (
36        46        IDENTIFIER          i
36        48        +                   +
36        50        CONSTANT            1
36        51        )                   )
36        52        )                   )
36        53        ;                   ;
38        9         IDENTIFIER          printf
38        15        (                   (
38        16        STRING_LITERAL      "\nResult via std\n"
38        36        )                   )
38        37        ;                   ;
39        9         IDENTIFIER          printf
39        15        (                   (
39        16        STRING_LITERAL      "ID: %s\n"
39        26        ,                   ,
39        28        IDENTIFIER          std
39        31        [                   [
39        32        IDENTIFIER          i
39        33        ]                   ]
39        34        .                   .
39        35        IDENTIFIER          id
39        37        )                   )
39        38        ;                   ;
40        9         IDENTIFIER          printf
40        15        (                   (
40        16        STRING_LITERAL      "First Name: %s\n"
40        34        ,                   ,
40        36        IDENTIFIER          std
40        39        [                   [
40        40        IDENTIFIER          i
40        41        ]                   ]
40        42        .                   .
40        43        IDENTIFIER          firstn
40        49        )                   )
40        50        ;                   ;
41        9         IDENTIFIER          printf
41        15        (                   (
41        16        STRING_LITERAL      "Last Name: %s\n"
41        33        ,                   ,
41        35        IDENTIFIER          std
41        38        [                   [
41        39        IDENTIFIER          i
41        40        ]                   ]
41        41        .                   .
41        42        IDENTIFIER          lastn
41        47        )                   )
41        48        ;                   ;
42        9         IDENTIFIER          printf
42        15        (                   (
42        16        STRING_LITERAL      "Points: %f\n"
42        30        ,                   ,
42        32        IDENTIFIER          std
42        35        [                   [
42        36        IDENTIFIER          i
42        37        ]                   ]
42        38        .                   .
42        39        IDENTIFIER          score
42        44        )                   )
42        45        ;                   ;
44        9         IDENTIFIER          printf
44        15        (                   (
44        16        STRING_LITERAL      "\nResult via ptr\n"
44        36        )                   )
44        37        ;                   ;
45        9         IDENTIFIER          printf
45        15        (                   (
45        16        STRING_LITERAL      "ID: %s\n"
45        26        ,                   ,
45        28        IDENTIFIER          ptr
45        31        PTR_OP              ->
45        33        IDENTIFIER          id
45        35        )                   )
45        36        ;                   ;
46        9         IDENTIFIER          printf
46        15        (                   (
46        16        STRING_LITERAL      "First Name: %s\n"
46        34        ,                   ,
46        36        IDENTIFIER          ptr
46        39        PTR_OP              ->
46        41        IDENTIFIER          firstn
46        47        )                   )
46        48        ;                   ;
47        9         IDENTIFIER          printf
47        15        (                   (
47        16        STRING_LITERAL      "Last Name: %s\n"
47        33        ,                   ,
47        35        IDENTIFIER          ptr
47        38        PTR_OP              ->
47        40        IDENTIFIER          lastn
47        45        )                   )
47        46        ;                   ;
48        9         IDENTIFIER          printf
48        15        (                   (
48        16        STRING_LITERAL      "Points: %f\n"
48        30        ,                   ,
48        32        IDENTIFIER          ptr
48        35        PTR_OP              ->
48        37        IDENTIFIER          score
48        42        )                   )
48        43        ;                   ;
50        9         IDENTIFIER          ptr
50        12        DEC_OP              --
50        14        ;                   ;
51        5         }                   }
53        5         RETURN              return
53        12        CONSTANT            0
53        13        ;                   ;
54        1         }                   }
*/