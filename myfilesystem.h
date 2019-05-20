#ifndef MYFILESYSTEM_H
#define MYFILESYSTEM_H
#include <sys/types.h>
#include <stdint.h>

void * init_fs(char * f1, char * f2, char * f3, int n_processors);

void close_fs(void * helper);

int create_file(char * filename, size_t length, void * helper);

int resize_file(char * filename, size_t length, void * helper);

void repack(void * helper);

int delete_file(char * filename, void * helper);

int rename_file(char * oldname, char * newname, void * helper);

int read_file(char * filename, size_t offset, size_t count, void * buf, void * helper);

int write_file(char * filename, size_t offset, size_t count, void * buf, void * helper);

ssize_t file_size(char * filename, void * helper);

void fletcher(uint8_t * buf, size_t length, uint8_t * output);

void compute_hash_tree(void * helper);

void compute_hash_block(size_t block_offset, void * helper);

#endif

#define MAX_LENGTH_FILE_NAME (63)
#define MAXN (24)
#define N (4)

typedef struct Block { //256
    char byte[256];
} Block;
typedef struct Directory { //72
    char file_name[64];
    int offset;
    int length;
    int index;
} Directory;
typedef struct Node { //16
    char byte[16];
} Node;
struct Helper {
    char f1[63], f2[63], f3[63];
    unsigned long int no_directory, no_block, no_node;
    unsigned long int size_file,size_hash,size_directory;
    char* file_data;
    Node* hash_data;
    Directory* directory_table;
    Directory* real_directory;
} helper;
