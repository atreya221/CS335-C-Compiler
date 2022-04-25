void printi(int c);
void printc(char c);
void prints(char *str);
void scans(char* str, int n);
int scani();
int open_file(char *name, int flags, int mode);
int read_file(int fd, char *buf, int max_length);
int write_file(int fd, char *buf, int len);
void close_file(int fd);
int strlen(char *str);

int main(){

    int fd,fd2, fd3;
    char buf[10000];
    char *input;
    
    input="README.md";
    fd=open_file(input,0,0);
    read_file(fd, buf, 10000);
    close_file(fd);


    fd2=open_file("b.txt",0x41,0x1ff);
    printi(fd2);
    printc('\n');
    write_file(fd2,"hello_world",9);
    close_file(fd2);

    fd3=open_file("extra.md",0x41,0x1ff);
    printi(fd3);
    printc('\n');
    printi(write_file(fd3, buf, strlen(buf)));
    printc('\n');
    
    close_file(fd3);
    return 0;
}