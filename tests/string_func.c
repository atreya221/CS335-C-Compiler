void printi(int c);
void printc(char c);
void prints(char *str);
void scans(char* str, int n);
int scani();
int strlen(char *str);
int str_index_of(char *str, char c);
int substr(char *str, char *search_str);
int strncpy(char *dest, char *src, int b);
int strcmp(char *first, char *second);
int strncmp(char *first, char *second, int n);
void memcpy(char *dest, char *src, int num);
void memset(char *buf, char b, int n);
int atoi(char *str);
int htoi(char *str);
void str_append(char *prefix, char *suffix, char *dest);
void str_nappend(char *prefix, char *suffix, char *dest, int n);

int main(){
    char *arr,*arr2,*arr3,*arr4,*arr5;
    char arr6[20],arr7[20],arr8[20],arr10[20];
    arr="hello_world";
    arr2="ell";

    prints("\nTesting strlen\n");
    printi(strlen(arr));
    prints("\nTesting str_index_of\n");
    printi(str_index_of(arr,'a'));

    prints("\nTesting substr\n");
    printi(substr(arr,arr2));
    prints("\nTesting strncpy\n");
    strncpy(arr6,arr,5);
    prints(arr6);

    printi(strcmp(arr6,arr2));
    printc('\n');
    printi(strcmp(arr6,"hell"));

    printi(strncmp(arr6,arr2,4));
    printc('\n');
    printi(strncmp(arr6,arr,4));

    prints("\nTesting memcpy\n");
    memcpy(arr7,arr,3);
    prints(arr);
    printc('\n');
    prints(arr7);

    prints("\nTesting atoi\n");
    arr4="1023";
    printi(atoi(arr4));

    prints("\nTesting htoi\n");
    arr4="23";
    printi(htoi(arr4));

    prints("\nTesting str_append\n");
    str_append(arr6,arr,arr8);
    prints(arr8);

    prints("\n");
}