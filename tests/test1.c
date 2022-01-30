#include <stdio.h>

int x = 0x1c;

void quick_sort (long long int *data, long long int n) 
    {
        long long int i, j, p, t;
        if (n < 2)
            return;
        p = data[n / 2];
        for (i = 0, j = n - 1;; i++, j--) {
            while (data[i] < p)
                i++;
            while (p < data[j])
                j--;
            if (i >= j)
                break;
            t = data[i];
            data[i] = data[j];
            data[j] = t;
        }
        quick_sort(data, i);
        quick_sort(data + i, n - i);
        return;
    }
    
int main(void) {
        long int t; scanf("%ld",&t);
        while(t--)
        {
            long long int c[3],k; 
            scanf("%lld%lld%lld",&c[0],&c[1],&c[2]);
            quick_sort(c,3);
            scanf("%lld",&k);
            printf("hello");
            if(k<=c[0]) printf("%lld\n",(k-1)*3+1);
            else if(k>c[0]&&k<=c[1]) printf("%lld\n",c[0]*3+(k-1-c[0])*2+1);
            else if(k>c[1]&&k<=c[2]) printf("%lld\n",c[0]+k+c[1]);
        }
    	return 0;
    }



// Line#     Column#   Token               Lexeme                        
// ======================================================================
// 1         1         IDENTIFIER          include
// 1         9         <                   <
// 1         10        IDENTIFIER          stdio
// 1         15        .                   .
// 1         16        IDENTIFIER          h
// 1         17        >                   >
// 3         1         VOID                void
// 3         6         IDENTIFIER          quick_sort
// 3         17        (                   (
// 3         18        LONG                long
// 3         23        LONG                long
// 3         28        INT                 int
// 3         32        *                   *
// 3         33        IDENTIFIER          data
// 3         37        ,                   ,
// 3         39        LONG                long
// 3         44        LONG                long
// 3         49        INT                 int
// 3         53        IDENTIFIER          n
// 3         54        )                   )
// 4         5         {                   {
// 5         9         LONG                long
// 5         14        LONG                long
// 5         19        INT                 int
// 5         23        IDENTIFIER          i
// 5         24        ,                   ,
// 5         26        IDENTIFIER          j
// 5         27        ,                   ,
// 5         29        IDENTIFIER          p
// 5         30        ,                   ,
// 5         32        IDENTIFIER          t
// 5         33        ;                   ;
// 6         9         IF                  if
// 6         12        (                   (
// 6         13        IDENTIFIER          n
// 6         15        <                   <
// 6         17        CONSTANT            2
// 6         18        )                   )
// 7         13        RETURN              return
// 7         19        ;                   ;
// 8         9         IDENTIFIER          p
// 8         11        =                   =
// 8         13        IDENTIFIER          data
// 8         17        [                   [
// 8         18        IDENTIFIER          n
// 8         20        /                   /
// 8         22        CONSTANT            2
// 8         23        ]                   ]
// 8         24        ;                   ;
// 9         9         FOR                 for
// 9         13        (                   (
// 9         14        IDENTIFIER          i
// 9         16        =                   =
// 9         18        CONSTANT            0
// 9         19        ,                   ,
// 9         21        IDENTIFIER          j
// 9         23        =                   =
// 9         25        IDENTIFIER          n
// 9         27        -                   -
// 9         29        CONSTANT            1
// 9         30        ;                   ;
// 9         31        ;                   ;
// 9         33        IDENTIFIER          i
// 9         34        INC_OP              ++
// 9         36        ,                   ,
// 9         38        IDENTIFIER          j
// 9         39        DEC_OP              --
// 9         41        )                   )
// 9         43        {                   {
// 10        13        WHILE               while
// 10        19        (                   (
// 10        20        IDENTIFIER          data
// 10        24        [                   [
// 10        25        IDENTIFIER          i
// 10        26        ]                   ]
// 10        28        <                   <
// 10        30        IDENTIFIER          p
// 10        31        )                   )
// 11        17        IDENTIFIER          i
// 11        18        INC_OP              ++
// 11        20        ;                   ;
// 12        13        WHILE               while
// 12        19        (                   (
// 12        20        IDENTIFIER          p
// 12        22        <                   <
// 12        24        IDENTIFIER          data
// 12        28        [                   [
// 12        29        IDENTIFIER          j
// 12        30        ]                   ]
// 12        31        )                   )
// 13        17        IDENTIFIER          j
// 13        18        DEC_OP              --
// 13        20        ;                   ;
// 14        13        IF                  if
// 14        16        (                   (
// 14        17        IDENTIFIER          i
// 14        19        GE_OP               >=
// 14        22        IDENTIFIER          j
// 14        23        )                   )
// 15        17        BREAK               break
// 15        22        ;                   ;
// 16        13        IDENTIFIER          t
// 16        15        =                   =
// 16        17        IDENTIFIER          data
// 16        21        [                   [
// 16        22        IDENTIFIER          i
// 16        23        ]                   ]
// 16        24        ;                   ;
// 17        13        IDENTIFIER          data
// 17        17        [                   [
// 17        18        IDENTIFIER          i
// 17        19        ]                   ]
// 17        21        =                   =
// 17        23        IDENTIFIER          data
// 17        27        [                   [
// 17        28        IDENTIFIER          j
// 17        29        ]                   ]
// 17        30        ;                   ;
// 18        13        IDENTIFIER          data
// 18        17        [                   [
// 18        18        IDENTIFIER          j
// 18        19        ]                   ]
// 18        21        =                   =
// 18        23        IDENTIFIER          t
// 18        24        ;                   ;
// 19        9         }                   }
// 20        9         IDENTIFIER          quick_sort
// 20        19        (                   (
// 20        20        IDENTIFIER          data
// 20        24        ,                   ,
// 20        26        IDENTIFIER          i
// 20        27        )                   )
// 20        28        ;                   ;
// 21        9         IDENTIFIER          quick_sort
// 21        19        (                   (
// 21        20        IDENTIFIER          data
// 21        25        +                   +
// 21        27        IDENTIFIER          i
// 21        28        ,                   ,
// 21        30        IDENTIFIER          n
// 21        32        -                   -
// 21        34        IDENTIFIER          i
// 21        35        )                   )
// 21        36        ;                   ;
// 22        9         RETURN              return
// 22        15        ;                   ;
// 23        5         }                   }
// 25        1         INT                 int
// 25        5         IDENTIFIER          main
// 25        9         (                   (
// 25        10        VOID                void
// 25        14        )                   )
// 25        16        {                   {
// 26        9         LONG                long
// 26        14        INT                 int
// 26        18        IDENTIFIER          t
// 26        19        ;                   ;
// 26        21        IDENTIFIER          scanf
// 26        26        (                   (
// 26        27        %                   %
// 26        28        IDENTIFIER          ld
// 26        30        ,                   ,
// 26        31        &                   &
// 26        32        IDENTIFIER          t
// 26        33        )                   )
// 26        34        ;                   ;
// 27        9         WHILE               while
// 27        14        (                   (
// 27        15        IDENTIFIER          t
// 27        16        DEC_OP              --
// 27        18        )                   )
// 28        9         {                   {
// 29        13        LONG                long
// 29        18        LONG                long
// 29        23        INT                 int
// 29        27        IDENTIFIER          c
// 29        28        [                   [
// 29        29        CONSTANT            3
// 29        30        ]                   ]
// 29        31        ,                   ,
// 29        32        IDENTIFIER          k
// 29        33        ;                   ;
// 30        13        IDENTIFIER          scanf
// 30        18        (                   (
// 30        19        %                   %
// 30        20        IDENTIFIER          lld
// 30        23        %                   %
// 30        24        IDENTIFIER          lld
// 30        27        %                   %
// 30        28        IDENTIFIER          lld
// 30        31        ,                   ,
// 30        32        &                   &
// 30        33        IDENTIFIER          c
// 30        34        [                   [
// 30        35        CONSTANT            0
// 30        36        ]                   ]
// 30        37        ,                   ,
// 30        38        &                   &
// 30        39        IDENTIFIER          c
// 30        40        [                   [
// 30        41        CONSTANT            1
// 30        42        ]                   ]
// 30        43        ,                   ,
// 30        44        &                   &
// 30        45        IDENTIFIER          c
// 30        46        [                   [
// 30        47        CONSTANT            2
// 30        48        ]                   ]
// 30        49        )                   )
// 30        50        ;                   ;
// 31        13        IDENTIFIER          quick_sort
// 31        23        (                   (
// 31        24        IDENTIFIER          c
// 31        25        ,                   ,
// 31        26        CONSTANT            3
// 31        27        )                   )
// 31        28        ;                   ;
// 32        13        IDENTIFIER          scanf
// 32        18        (                   (
// 32        19        %                   %
// 32        20        IDENTIFIER          lld
// 32        23        ,                   ,
// 32        24        &                   &
// 32        25        IDENTIFIER          k
// 32        26        )                   )
// 32        27        ;                   ;
// 33        13        IF                  if
// 33        15        (                   (
// 33        16        IDENTIFIER          k
// 33        17        LE_OP               <=
// 33        19        IDENTIFIER          c
// 33        20        [                   [
// 33        21        CONSTANT            0
// 33        22        ]                   ]
// 33        23        )                   )
// 33        25        IDENTIFIER          printf
// 33        31        (                   (
// 33        32        %                   %
// 33        33        IDENTIFIER          lld
// 33        36        IDENTIFIER          n
// 33        37        ,                   ,
// 33        38        (                   (
// 33        39        IDENTIFIER          k
// 33        40        -                   -
// 33        41        CONSTANT            1
// 33        42        )                   )
// 33        43        *                   *
// 33        44        CONSTANT            3
// 33        45        +                   +
// 33        46        CONSTANT            1
// 33        47        )                   )
// 33        48        ;                   ;
// 34        13        ELSE                else
// 34        18        IF                  if
// 34        20        (                   (
// 34        21        IDENTIFIER          k
// 34        22        >                   >
// 34        23        IDENTIFIER          c
// 34        24        [                   [
// 34        25        CONSTANT            0
// 34        26        ]                   ]
// 34        27        AND_OP              &&
// 34        29        IDENTIFIER          k
// 34        30        LE_OP               <=
// 34        32        IDENTIFIER          c
// 34        33        [                   [
// 34        34        CONSTANT            1
// 34        35        ]                   ]
// 34        36        )                   )
// 34        38        IDENTIFIER          printf
// 34        44        (                   (
// 34        45        %                   %
// 34        46        IDENTIFIER          lld
// 34        49        IDENTIFIER          n
// 34        50        ,                   ,
// 34        51        IDENTIFIER          c
// 34        52        [                   [
// 34        53        CONSTANT            0
// 34        54        ]                   ]
// 34        55        *                   *
// 34        56        CONSTANT            3
// 34        57        +                   +
// 34        58        (                   (
// 34        59        IDENTIFIER          k
// 34        60        -                   -
// 34        61        CONSTANT            1
// 34        62        -                   -
// 34        63        IDENTIFIER          c
// 34        64        [                   [
// 34        65        CONSTANT            0
// 34        66        ]                   ]
// 34        67        )                   )
// 34        68        *                   *
// 34        69        CONSTANT            2
// 34        70        +                   +
// 34        71        CONSTANT            1
// 34        72        )                   )
// 34        73        ;                   ;
// 35        13        ELSE                else
// 35        18        IF                  if
// 35        20        (                   (
// 35        21        IDENTIFIER          k
// 35        22        >                   >
// 35        23        IDENTIFIER          c
// 35        24        [                   [
// 35        25        CONSTANT            1
// 35        26        ]                   ]
// 35        27        AND_OP              &&
// 35        29        IDENTIFIER          k
// 35        30        LE_OP               <=
// 35        32        IDENTIFIER          c
// 35        33        [                   [
// 35        34        CONSTANT            2
// 35        35        ]                   ]
// 35        36        )                   )
// 35        38        IDENTIFIER          printf
// 35        44        (                   (
// 35        45        %                   %
// 35        46        IDENTIFIER          lld
// 35        49        IDENTIFIER          n
// 35        50        ,                   ,
// 35        51        IDENTIFIER          c
// 35        52        [                   [
// 35        53        CONSTANT            0
// 35        54        ]                   ]
// 35        55        +                   +
// 35        56        IDENTIFIER          k
// 35        57        +                   +
// 35        58        IDENTIFIER          c
// 35        59        [                   [
// 35        60        CONSTANT            1
// 35        61        ]                   ]
// 35        62        )                   )
// 35        63        ;                   ;
// 36        9         }                   }
// 37        8         RETURN              return
// 37        15        CONSTANT            0
// 37        16        ;                   ;
// 38        5         }                   }