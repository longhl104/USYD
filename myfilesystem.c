#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "myfilesystem.h"

char* my_truncate(char* a) {
    if(sizeof(a)>MAX_LENGTH_FILE_NAME) {
        char* b = (char*) malloc(MAX_LENGTH_FILE_NAME);
        for(int i = 0;i<MAX_LENGTH_FILE_NAME;++i)
            b[i] = a[i];
        return b;
    }
    return a;
}
int compare_directory(const void *s1, const void *s2)
{
    Directory *e1 = (Directory*)s1;
    Directory *e2 = (Directory*)s2;
    return e1->offset - e2->offset;
}
void * init_fs(char * f1, char * f2, char * f3, int n_processors) {
    f1 = my_truncate(f1);
    f2 = my_truncate(f2);
    f3 = my_truncate(f3);
    if (f1 == f2 || f2 == f3 || f3 == f1) {
        printf("%s\n", strerror(errno));
        return NULL;
    }
    strcpy(helper.f1,f1);
    strcpy(helper.f2,f2);
    strcpy(helper.f3,f3);

    FILE *fptr;
    if ((fptr = fopen(f1,"rb")) == NULL) {
        printf("%s\n", strerror(errno));
        return NULL;
    }
    helper.size_file = 0;
    char dummy;
    while(fread(&dummy, sizeof(char), 1, fptr)) {
        helper.file_data[helper.size_file++] = dummy;
    }
    helper.no_block = helper.size_file/256;
    fclose(fptr);
    //////////////////////////////////////////////////
    if ((fptr = fopen(f2,"rb")) == NULL) {
        printf("%s\n", strerror(errno));
        return NULL;
    }
    Directory dummy2;
    helper.no_directory = 0;
    helper.size_directory = 0;
    while(fread(&dummy2, 72, 1, fptr)) {
        if(strcmp(dummy2.file_name,"")!=0||dummy2.offset!=0||dummy2.length!=0) {
            helper.real_directory[helper.no_directory++] = dummy2;
            helper.real_directory[helper.no_directory-1].index = helper.size_directory;
        }
        helper.directory_table[helper.size_directory++] = dummy2;
    }
    qsort(helper.real_directory,helper.no_directory,sizeof(Directory),compare_directory);
    fclose(fptr);
    // ////////////////////////////////////////////////////
    if ((fptr = fopen(f3,"rb")) == NULL) {
        printf("%s\n", strerror(errno));
        return NULL;
    }
    Node dummy3;
    helper.size_hash = 0;
    while(fread(&dummy3, sizeof(Node), 1, fptr)) {
        helper.hash_data[helper.size_hash++] = dummy3;
    }
    fclose(fptr);

    return (void*)&helper;
}

void close_fs(void * helper) {
    // ((struct Helper*)helper)->no_directory = 0;
    // ((struct Helper*)helper)->size_directory = 0;
    // ((struct Helper*)helper)->size_file = 0;
    // ((struct Helper*)helper)->no_block = 0;
    // ((struct Helper*)helper)->no_node = 0;
    // ((struct Helper*)helper)->size_hash = 0;
    return;
}

void write_data(void* helper) {
    FILE* fptr;
    fptr = fopen(((struct Helper *)helper)->f1, "wb");
    fwrite(((struct Helper *)helper)->file_data, sizeof(char), ((struct Helper *)helper)->size_file, fptr);
    fclose(fptr);
    fptr = fopen(((struct Helper *)helper)->f2, "wb");
    fwrite(((struct Helper *)helper)->directory_table, 72, ((struct Helper *)helper)->size_directory, fptr);
    fclose(fptr);
    fptr = fopen(((struct Helper *)helper)->f3, "wb");
    fwrite(((struct Helper *)helper)->hash_data, sizeof(Node), ((struct Helper *)helper)->size_hash, fptr);
    fclose(fptr);
}
void assign_data(int offset, int length, void* helper) {
    for(int i=0;i<length;++i)
        ((struct Helper*)helper)->file_data[offset+i] = '\0';
}
int is_empty_directory(Directory* a) {
    if(strcmp(a->file_name,"") == 0&& a->offset==0 && a->length==0)
        return 1;
    return 0;
}
int create_file(char * filename, size_t length, void * helper) {
    (struct Helper*)h = (struct Helper*)helper;
    filename = my_truncate(filename);
    for(int i=0;i<((struct Helper *)helper)->size_directory;++i) {
        if(strcmp(((struct Helper *)helper)->directory_table[i].file_name, filename) == 0) {
            return 1;
        }
    }

    int res = -1;

    if(length <= h->real_directory[0].offset)
        res = 0;
    else {
        for(int i=0;i< h->no_directory - 1;++i) {
            if(h->real_directory[i].offset + h->real_directory[i].length + length <= h->real_directory[i+1].offset) {
                res = h->real_directory[i].offset + h->real_directory[i].length;
                break;
            }
        }
        if(res == -1) {
            if(h->no_directory>0)
                if(h->real_directory[h->no_directory - 1].offset + h->real_directory[h->no_directory - 1].length + length <= h->size_file)
                res = h->real_directory[h->no_directory - 1].offset + h->real_directory[h->no_directory - 1].length;
        }
    }
    if(res == -1 || h->size_directory == h->no_directory)
        return 2;
    for(int i=0;i<h->size_directory;++i)
        if(is_empty_directory(h->directory_table[i])) {
            h->directory_table[i].offset = res;
            h->directory_table[i].length = length;
            strcpy(h->directory_table[i].file_name, filename);
            h->no_directory++;
            h->real_directory[h->no_directory-1] = h->directory_table[i];
            h->real_directory[h->no_directory-1].index = i;
            qsort(h->real_directory,h->no_directory,sizeof(Directory),compare_directory);
            assign_data(res,length,helper);
            break;
        }
    write_data(helper);
    return 0;
}

int file_exist(char* filename, void *helper) {
    (struct Helper*)h = (struct Helper*)helper;
    for(int i=0;i<h->no_directory;++i)
        if(strcmp(filename, h->real_directory[i].file_name ) == 0)
            return i;
    return -1;
}
void swap_char(char* a, int x, int y) {
    char t = a[x];
    a[x] = a[y];
    a[y] = t;
}
int resize_file(char * filename, size_t length, void * helper) {
    (struct Helper*)h = (struct Helper*)helper;
    filename = my_truncate(filename);
    int pos = file_exist(filename, helper);
    if(pos == -1)
        return 1;
    int res = 2;
    if(length<=h->real_directory[pos].length) {;
        h->real_directory[pos].length = length;
        h->directory_table[h->real_directory[pos].index].length = length;
        res = 0;
    } else {
        // Directory my_directory_table[h->no_directory];
        // for(int i=0, j=0;i<h->size_directory;++i) {
        //     if(is_empty_directory( &h->directory_table[i]) ==0 ) {
        //         my_directory_table[j++] = h->directory_table[i];
        //     }
        // }
        // qsort(my_directory_table,h->no_directory,sizeof(Directory),compare_directory);
        if(pos==h->no_directory-1) {
            if(h->real_directory[pos].offset + length <= h->size_file) {
                h->real[pos].length = length;
                h->directory_table[h->real_directory[pos].index].length = length;
                assign_data(h->real_directory[i].offset+h->real_directory[i].length, length -  h->real_directory[i].length, helper);
                res = 0;
            }
        } else {
            if(h->real_directory[pos].offset + length <= h->real_directory[pos+1].offset) {
                h->real[pos].length = length;
                h->directory_table[h->real_directory[pos].index].length = length;
                assign_data(h->real_directory[i].offset+h->real_directory[i].length, length -  h->real_directory[i].length, helper);
                res = 0;
            }
        }
        if(res == 2) {
            int total = 0;
            struct Helper other = *helper;
            other.real_directory[pos].file_name = "";
            other.real_directory[pos].length = 0;
            other.real_directory[pos].offset = 0;
            other.directory_table[other.real_directory[pos].index].file_name = "";
            other.directory_table[other.real_directory[pos].index].offset = 0;
            other.directory_table[other.real_directory[pos].index].length = 0;
            if(total + length <= h->size_file) {
                h->directory_table[pos].offset = total;
                h->directory_table[pos].length = length;
                int pos = 0;
                for(int i=0;i<h->no_directory;++i)
                    if(strcmp(h->real_directory[i].file_name, h->directory_table[pos].file_name)!=0) {
                        for(int j=0;j<h->real_directory[i].length;++j) {
                            swap_char(h->file_data, pos+j, h->real_directory[i].offset+j);
                        }
                        h->real_directory[i].offset = pos;
                        pos+=h->real_directory[i].length;
                    } else {
                        for(int j=0;j<h->real_directory[i].length;++j) {
                            swap_char(h->file_data, total+j, h->real_directory[i].offset+j);
                        }
                        assign_data(total+h->real_directory[i].length, length - h->real_directory[i].length, helper);
                        h->real_directory[i].offset = total;
                        h->real_directory[i].length = length;
                    }
                for(int i=0;i<h->size_directory;++i)
                    for(int j=0;j<h->no_directory;++j)
                        if(strcmp(h->real_directory[j].file_name, h->directory_table[i].file_name) == 0) {
                            h->directory_table[i] = h->real_directory[j];
                        }
                res = 0;
            }
        }
    }
    if(res == 0)
        write_data(helper);
    return res;
}

void repack(void * helper) {
    (struct Helper*)h = (struct Helper*)helper;
    // Directory my_directory_table[((struct Helper *)helper)->no_directory];
    int pos = 0;
    for(int i=0; i<h->no_directory; ++i) {
        for(int j=0;j<h->real_directory[i].length;++j) {
            swap_char(h->file_data, pos+j, h->real_directory[i].offset+j);
        }
        h->real_directory[i].offset = pos;
        h->directory_table[h->real_directory[i].index].offset = pos
        pos+=h->real_directory[i].length;
    }
    write_data(helper);
    return;
}

int delete_file(char * filename, void * helper) {
    int index = file_exist(filename, helper);
    if(index==-1)
        return 1;
    strcpy(h->directory_table[index].file_name,"");
    h->directory_table[index].offset = 0;
    h->directory_table[index].length = 0;
    write_data(helper);
    return 0;
}

int rename_file(char * oldname, char * newname, void * helper) {
    int index = file_exist(oldname, helper);
    if(index==-1 || file_exist(newname, helper) != -1)
        return 1;
    strcpy(((struct Helper *)helper)->directory_table[index].file_name,newname);
    write_data(helper);
    return 0;
}

int read_file(char * filename, size_t offset, size_t count, void * buf, void * helper) {
    int index = file_exist(filename, helper);
    if(index == -1)
        return 1;
    if(offset<0 || offset >= ((struct Helper *)helper)->directory_table[index].length)
        return 2;
    if(offset + count > ((struct Helper *)helper)->directory_table[index].length)
        return 2;
    FILE* fptr;
    char dummy;
    fptr = fopen(((struct Helper *)helper)->f1, "rb");
    for(int i=0;i<((struct Helper *)helper)->directory_table[index].offset + offset;++i)
        fread(&dummy, sizeof(char), 1, fptr);
    fread(buf, sizeof(char), count, fptr);
    fclose(fptr);
    return 0;
}

int write_file(char * filename, size_t offset, size_t count, void * buf, void * helper) {
    char* buff = (char*) buf;
    int index = file_exist(filename, helper);
    if(index == -1)
        return 1;
    if(offset >= ((struct Helper *)helper)->directory_table[index].length)
        return 2;
    FILE* fptr;
    fptr = fopen(((struct Helper *)helper)->f1, "rb");
    if(offset + count >= ((struct Helper *)helper)->directory_table[index].length) {
        if(resize_file(filename, count, helper) == 2)
            return 3;
        for(int i=0;i<count;++i) {
            ((struct Helper *)helper)->file_data[((struct Helper *)helper)->directory_table[index].offset+offset+i] = *(buff+i);
        }
    } else {
        for(int i=0;i<count;++i) {
            ((struct Helper *)helper)->file_data[((struct Helper *)helper)->directory_table[index].offset+offset+i] = *(buff+i);
        }
    }

    fptr = fopen(((struct Helper *)helper)->f1, "wb");
    fwrite(((struct Helper *)helper)->file_data, sizeof(char), ((struct Helper *)helper)->size_file, fptr);
    fclose(fptr);
    return 0;
}

ssize_t file_size(char * filename, void * helper) {
    for(int i=0;i<((struct Helper *)helper)->size_directory;++i) {
        if(strcmp(((struct Helper *)helper)->directory_table[i].file_name, filename) == 0)
            return ((struct Helper *)helper)->directory_table[i].length;
    }
    return -1;
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
