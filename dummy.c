#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LENGTH_FILE_NAME 5

char* truncate(char* a) {
    if(sizeof(a)>MAX_LENGTH_FILE_NAME) {
        char* b = (char*) malloc(MAX_LENGTH_FILE_NAME);
        for(int i = 0;i<MAX_LENGTH_FILE_NAME;++i)
            b[i] = a[i];
        return b;
    }
    return a;
}

typedef struct Directory {
    char file_name[64];
    int offset;
    int length;
} Directory;
typedef struct Block {
    char byte[256];
} Block;
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

int main(int argc, char** argv) {
    printf("%d\n", argc);
    int a[4] = {1,2,3,4};
    int *b = a;
    *(b+2) = 5;
    for(int i=0;i<4;++i)
        printf("%ls\n", &(*(a+3)));
    // printf("%f\n", log(4)/log(2));
    return 0;
}
