void printi( int x);
void printc( char x);
void prints(char *);
int scani();

void bubble_sort(int *arr, int n)
{
   int i, j;
   for (i = 0; i < n-1; i++)     
 
       for (j = 0; j < n-i-1; j++)
           if (arr[j] > arr[j+1]){               
               int temp = arr[j];
               arr[j] = arr[j+1];
               arr[j+1] = temp;
           }
    return;
}
 
void print_array(int *arr, int size)
{
    int i;
    for (i=0; i < size; i++){
        printi(arr[i]);
        printc(' ');
    }
    printc('\n');
}
 
int main()
{
    int arr[50];
    int n, i;
    prints("Enter the number of elements: ");
    n=scani();
    prints("Enter the array elements\n");
    i = 0;
    while(i < n){
        arr[i] = scani();
        i++;
    }
    prints("Input array: \n");
    print_array(arr, n);

    bubble_sort(arr, n);
    
    prints("Sorted array: \n");
    print_array(arr, n);
    
    return 0;
}