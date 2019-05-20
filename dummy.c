#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "myfilesystem.h"


char* truncate(char* a) {
    if(sizeof(a)>MAX_LENGTH_FILE_NAME) {
        char* b = (char*) malloc(MAX_LENGTH_FILE_NAME);
        for(int i = 0;i<MAX_LENGTH_FILE_NAME;++i)
            b[i] = a[i];
        return b;
    }
    return a;
}

void swap_char(char* a, int x, int y) {
    char t = a[x];
    a[x] = a[y];
    a[y] = t;
}
void dump(void* a) {
    char* b = (char*) a;
    for(int i=0;i<3;++i) {
        printf("%c\n", *(b+i));
    }

}

void f(void* a) {
    int* a = (int*) a;
    printf("%d\n", *(a+1));
}

struct abc {
    int a;
    int b;
    int c;
};

int main(int argc, char** argv) {
    int a[4] = {4,3,2,5};
    f((void*)a);
    return 0;
}
