void printi(int c);
void printc(char c);
int scani();
void prints(char *str);

int binary_search(int  arr[100],int low, int high,int to_find){
    int mid,i;
    if(low>high){
        return -1;
    }
    else if(low==high){
        if(arr[low]==to_find){
            return low;
        }
        return -1;
    }
    else {
        mid = (low+high)/2;
        if(arr[mid]>to_find){
            return binary_search(arr,low,mid-1,to_find);
        }
        else if(arr[mid]==to_find){
            return mid;
        }
        else {
            return binary_search(arr,mid+1,high,to_find);
        }
    }
    return -1;
}
void print_array(int arr[100]){
    int x;
    for(x=0;x<50;x++){
        printi(arr[x]);
        printc(' ');
    }
    printc('\n');
    return;
}

int main(){
    int i,x,y,z;
    int arr[100];
    x=10;
    for(i=0;i<50;i++){
        arr[i]=x;
        x+=i;
    }
    print_array(arr);
    prints("Enter the number you want to search: ");
    z=scani();
    y=binary_search(arr,0,49,z);
    printi(y);
    prints("\n");
    return 0;
}

