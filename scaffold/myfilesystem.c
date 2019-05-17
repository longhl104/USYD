#include <stdlib.h>

#include "myfilesystem.h"

// Fill out the functions below

void * init_fs(char * f1, char * f2, char * f3, int n_processors) {
    return NULL;
}

void close_fs(void * helper) {
    return;
}

int create_file(char * filename, size_t length, void * helper) {
    return 0;
}

int resize_file(char * filename, size_t length, void * helper) {
    return 0;
}

void repack(void * helper) {
    return;
}

int delete_file(char * filename, void * helper) {
    return 0;
}

int rename_file(char * oldname, char * newname, void * helper) {
    return 0;
}

int read_file(char * filename, size_t offset, size_t count, void * buf, void * helper) {
    return 0;
}

int write_file(char * filename, size_t offset, size_t count, void * buf, void * helper) {
    return 0;
}

ssize_t file_size(char * filename, void * helper) {
    return 0;
}

void fletcher(uint8_t * buf, size_t length, uint8_t * output) {
    return;
}

void compute_hash_tree(void * helper) {
    return;
}

void compute_hash_block(size_t block_offset, void * helper) {
    return;
}
