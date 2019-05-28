#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "myfilesystem.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

char* my_truncate(char* a) {
    if(sizeof(a)>MAX_LENGTH_FILE_NAME) {
        char* b = (char*) malloc(MAX_LENGTH_FILE_NAME+1);
        int i = 0;
        for(int i = 0;i<MAX_LENGTH_FILE_NAME;++i)
            *(b+i) = a[i];
        *(b+i) = '\0';
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
    // printf("init_fs\n");
    f1 = my_truncate(f1);
    f2 = my_truncate(f2);
    f3 = my_truncate(f3);
    if (strcmp(f1,f2) == 0 || strcmp(f2,f3) == 0 || strcmp(f3,f1) == 0) {
        printf("%s\n", strerror(errno));
        return (void*)&helper;;
    }
    strcpy(helper.f1,f1);
    strcpy(helper.f2,f2);
    strcpy(helper.f3,f3);

    FILE *fptr;
    if ((fptr = fopen(f1,"rb")) == NULL) {
        printf("%s\n", strerror(errno));
        return (void*)&helper;
    }
    fseek(fptr, 0L, SEEK_END);
    int sz = ftell(fptr);

    helper.file_data = (char*)malloc(ftell(fptr));
    helper.size_file = 0;
    char dummy;
    fclose(fptr);
    fptr = fopen(f1,"rb");
    while(sz>0 && fread(&dummy, sizeof(char), 1, fptr)) {
        helper.file_data[helper.size_file++] = dummy;
    }
    helper.no_block = helper.size_file/256;
    fclose(fptr);
    //////////////////////////////////////////////////
    if ((fptr = fopen(f2,"rb")) == NULL) {
        printf("%s\n", strerror(errno));
        return (void*)&helper;
    }
    fseek(fptr, 0L, SEEK_END);
    sz = ftell(fptr);
    helper.directory_table = malloc((sz/72)*76);
    helper.real_directory = malloc((sz/72)*76);
    Directory dummy2;
    helper.no_directory = 0;
    helper.size_directory = 0;
    fclose(fptr);
    fptr = fopen(f2,"rb");
    while(sz>0 && fread(&dummy2, 72, 1, fptr)) {
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
        return (void*)&helper;
    }

    fseek(fptr, 0L, SEEK_END);
    sz = ftell(fptr);
    helper.hash_data = (Node*)malloc(sz);

    Node dummy3;
    helper.size_hash = 0;
    fclose(fptr);
    fptr = fopen(f3,"rb");
    while(sz>0 && fread(&dummy3, sizeof(Node), 1, fptr)) {
        helper.hash_data[helper.size_hash++] = dummy3;
    }
    fclose(fptr);
    // for(int i=0;i<helper.size_directory;++i)
    //     //printf("%s %d %d %d\n", helper.directory_table[i].file_name, helper.directory_table[i].offset, helper.directory_table[i].length, helper.directory_table[i].index);
    helper.no_block = helper.size_file/256;
    unsigned long int n = (unsigned long int) log2(helper.no_block);
    helper.no_node = ((1UL<<(n+1)) - 1);
    helper.hash_calculated = 0;
    return (void*)&helper;
}
void write_data(void* helper) {

    FILE* fptr;
    fptr = fopen(((struct Helper *)helper)->f1, "wb");
    fwrite(((struct Helper *)helper)->file_data, sizeof(char), ((struct Helper *)helper)->size_file, fptr);
    fclose(fptr);

    fptr = fopen(((struct Helper *)helper)->f2, "wb");
    for(int i=0; i<((struct Helper *)helper)->size_directory;++i) {
        fwrite(((struct Helper *)helper)->directory_table + i, 72, 1, fptr);
    }

    fclose(fptr);
    fptr = fopen(((struct Helper *)helper)->f3, "wb");
    fwrite(((struct Helper *)helper)->hash_data, sizeof(Node), ((struct Helper *)helper)->size_hash, fptr);
    fclose(fptr);

    // for(int i=0;i<((struct Helper*)helper)->size_directory;++i)
        //printf("%s %d %d %d\n", ((struct Helper*)helper)->directory_table[i].file_name, ((struct Helper*)helper)->directory_table[i].offset, ((struct Helper*)helper)->directory_table[i].length, ((struct Helper*)helper)->directory_table[i].index);
    //printf("###########################\n");
}

void close_fs(void * helper) {
    // printf("close_fs\n");
    write_data(helper);
    struct Helper* h = (struct Helper*)helper;
    free(h->file_data);
    free(h->hash_data);
    free(h->directory_table);
    free(h->real_directory);
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
    // printf("create_file\n");

    //printf("create_file %s %ld\n", filename, length);
    struct Helper* h = (struct Helper*)helper;
    filename = my_truncate(filename);
    for(int i=0;i<h->size_directory;++i) {
        if(strcmp(h->directory_table[i].file_name, filename) == 0) {
            return 1;
        }
    }
    if(h->no_directory == h->size_directory)
        return 2;
    int res = -1;
    //printf("%ld %ld\n", h->no_directory, h->size_directory);
    if(h->no_directory ==0) {
        if(length<=h->size_file) {
            h->directory_table[0].offset = 0;
            h->directory_table[0].length = length;
            strcpy(h->directory_table[0].file_name, filename);
            h->no_directory++;
            h->real_directory[0] = h->directory_table[0];
            h->real_directory[0].index = 0;
            assign_data(0,length,helper);
            compute_hash_tree(helper);
            write_data(helper);
            //printf("fuck\n");
            return 0;
        } else return 2;
    }
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
            if(h->real_directory[h->no_directory - 1].offset + h->real_directory[h->no_directory - 1].length + length <= h->size_file)
                res = h->real_directory[h->no_directory - 1].offset + h->real_directory[h->no_directory - 1].length;
        }
    }
    if(res!=-1) {
        for(int i=0;i<h->size_directory;++i)
            if(is_empty_directory(&h->directory_table[i])) {
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
        compute_hash_tree(helper);
        write_data(helper);
        return 0;
    }
    int total = 0;
    for(int i=0;i<h->no_directory;++i)
        total += h->real_directory[i].length;
    if(total + length <=h->size_file) {
        repack(helper);
        res = h->real_directory[h->no_directory-1].offset + h->real_directory[h->no_directory-1].length;
        for(int i=0;i<h->size_directory;++i)
            if(is_empty_directory(&h->directory_table[i])) {
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
        compute_hash_tree(helper);
        write_data(helper);
        return 0;
    } else return 2;
}

int file_exist(char* filename, void *helper) {
    struct Helper* h = (struct Helper*)helper;
    for(int i=0;i<h->no_directory;++i)
        if(strcmp(filename, h->real_directory[i].file_name ) == 0)
            return i;
    return -1;;
}
void swap_char(char* a, int x, int y) {
    char t = a[x];
    a[x] = a[y];
    a[y] = t;
}
int resize_file(char * filename, size_t length, void * helper) {
    // printf("resize_file\n");

    //printf("resize_file %s %ld\n", filename, length);
    struct Helper* h = (struct Helper*)helper;
    filename = my_truncate(filename);
    int pos = file_exist(filename, helper);
    if(pos == -1)
        return 1;
    int res = 2;
    if(length<=h->real_directory[pos].length) {
        h->real_directory[pos].length = length;
        h->directory_table[h->real_directory[pos].index].length = length;
        res = 0;
    } else {
        if(pos==h->no_directory-1) {
            if(h->real_directory[pos].offset + length <= h->size_file) {
                h->real_directory[pos].length = length;
                h->directory_table[h->real_directory[pos].index].length = length;
                assign_data(h->real_directory[pos].offset+h->real_directory[pos].length, length - h->real_directory[pos].length, helper);
                res = 0;
            }
        } else {
            if(h->real_directory[pos].offset + length <= h->real_directory[pos+1].offset) {
                h->real_directory[pos].length = length;
                h->directory_table[h->real_directory[pos].index].length = length;
                assign_data(h->real_directory[pos].offset+h->real_directory[pos].length, length -  h->real_directory[pos].length, helper);
                res = 0;
            }
        }
        if(res == 2) {
            int total = 0;
            for(int i=0;i<h->no_directory;++i)
                if(i!=pos)
                    total += h->real_directory[i].length;
            if(total + length <= h->size_file) {
                char* tmp = (char*) malloc(h->real_directory[pos].length);
                for(int i=0;i<h->real_directory[pos].length;++i) {
                    tmp[i] = h->file_data[h->real_directory[pos].offset+i];
                }
                int old_pos = h->real_directory[pos].index;
                int old_length = h->real_directory[pos].length;
                //printf("begin create_file\n");
                delete_file(filename, helper);
                repack(helper);
                ///////////////////////ADD FILE//////////////////////////
                ++h->no_directory;
                strcpy(h->real_directory[h->no_directory-1].file_name, filename);
                h->real_directory[h->no_directory-1].offset = h->real_directory[h->no_directory-2].offset + h->real_directory[h->no_directory-2].length;
                h->real_directory[h->no_directory-1].length = length;
                h->real_directory[h->no_directory-1].index = old_pos;
                strcpy(h->directory_table[old_pos].file_name, filename);
                h->directory_table[old_pos].offset = h->real_directory[h->no_directory-1].offset;
                h->directory_table[old_pos].length = length;
                for(int i=0;i<old_length;++i) {
                    h->file_data[h->real_directory[h->no_directory-1].offset+i] = tmp[i];
                }
                //////////////////////////////////////////////////////////////////////
                assign_data(h->real_directory[h->no_directory-1].offset + old_length, length - old_length, helper);
                res = 0;
                free(tmp);
            }
        }
    }
    if(res == 0) {
        compute_hash_tree(helper);
        write_data(helper);
    }
    // for(int i=0;i<h->size_directory;++i)
    //     //printf("%s %d %d %d\n", h->directory_table[i].file_name, h->directory_table[i].offset, h->directory_table[i].length, h->directory_table[i].index);
    return res;
}

void repack(void * helper) {
    // printf("repack\n");

    //printf("repack\n");
    struct Helper* h = (struct Helper*)helper;
    // Directory my_directory_table[((struct Helper *)helper)->no_directory];
    int pos = 0;
    for(int i=0; i<h->no_directory; ++i) {
        for(int j=0;j<h->real_directory[i].length;++j) {
            swap_char(h->file_data, pos+j, h->real_directory[i].offset+j);
        }
        h->real_directory[i].offset = pos;
        h->directory_table[h->real_directory[i].index].offset = pos;
        pos+=h->real_directory[i].length;
    }
    compute_hash_tree(helper);
    write_data(helper);
    return;
}

int delete_file(char * filename, void * helper) {
    // printf("delete_file\n");

    // printf("delete_file %s\n", filename);
    struct Helper* h = (struct Helper*)helper;
    int pos = file_exist(filename, helper);
    if(pos==-1)
        return 1;
    if(h->no_directory>0)
    {
        strcpy(h->directory_table[h->real_directory[pos].index].file_name, "");
        h->directory_table[h->real_directory[pos].index].offset = 0;
        h->directory_table[h->real_directory[pos].index].length = 0;
        for(int i=pos+1;i<h->no_directory;++i)
            h->real_directory[i-1] = h->real_directory[i];
        --h->no_directory;
        write_data(helper);
        return 0;
    }
    return 1;
}

int rename_file(char * oldname, char * newname, void * helper) {
    // printf("rename_file\n");

    int index = file_exist(oldname, helper);
    if(index==-1 || file_exist(newname, helper) != -1)
        return 1;
    strcpy(((struct Helper *)helper)->directory_table[index].file_name,newname);
    write_data(helper);
    return 0;
}

int read_file(char * filename, size_t offset, size_t count, void * buf, void * helper) {
    // printf("read_file\n");

    //printf("%s %ld %ld\n", filename, offset, count);
    struct Helper* h = (struct Helper*)helper;
    int index = file_exist(filename, helper);
    if(index == -1)
        return 1;
    if(offset<0 || offset >= h->real_directory[index].length)
        return 2;
    if(offset + count > h->real_directory[index].length)
        return 2;
    Node* old_hash_data = malloc(sizeof(Node)*h->no_node);
    memcpy(old_hash_data, h->hash_data, sizeof(Node)*h->no_node);
    for(int i=(h->real_directory[index].offset+offset)/256;i<=(h->real_directory[index].offset+offset+count)/256;++i)
        compute_hash_block(i, helper);
    for(int i=0;i<h->no_node;++i)
        for(int j=0;j<16;++j)
            if(old_hash_data[i].byte[j]!=h->hash_data[i].byte[j]) {
                memcpy(h->hash_data, old_hash_data, sizeof(Node)*h->no_node);
                free(old_hash_data);
                return 3;
            }
    free(old_hash_data);
    FILE* fptr;
    char dummy;
    fptr = fopen(h->f1, "rb");
    for(int i=0;i<h->real_directory[index].offset + offset;++i)
        fread(&dummy, sizeof(char), 1, fptr);
    fread(buf, sizeof(char), count, fptr);
    fclose(fptr);

    return 0;
}

int write_file(char * filename, size_t offset, size_t count, void * buf, void * helper) {
    // printf("write_file\n");
    pthread_mutex_lock(&lock);
    struct Helper* h = (struct Helper*)helper;
    // printf("%s\n", h->file_data);
    // printf("write_file %s %ld %ld %s\n", filename, offset, count, (char*)buf);
    char* buff = (char*) buf;
    int index = file_exist(filename, helper);
    if(index == -1) {
        compute_hash_tree(helper);
        pthread_mutex_unlock(&lock);
        return 1;
    }
    if(offset > h->real_directory[index].length){
        compute_hash_tree(helper);
        pthread_mutex_unlock(&lock);
        return 2;
    }
    FILE* fptr;
    if(offset + count > h->real_directory[index].length) {
        // printf("jlala jfad\n");
        int total = 0;
        for(int i=0;i<h->no_directory;++i)
            if(i!=index)
                total += h->real_directory[i].length;
        if(total + offset + count > h->size_file) {
            compute_hash_tree(helper);
            pthread_mutex_unlock(&lock);
            return 3;
        }
        resize_file(filename, offset + count, helper);
        for(int i=0;i<count;++i) {
            index = file_exist(filename, helper);
            // printf("%ld\n", h->real_directory[index].offset+offset+i);
            h->file_data[h->real_directory[index].offset+offset+i] = *(buff+i);
        }
    } else {
        for(int i=0;i<count;++i) {
            // printf("%ld\n", h->real_directory[index].offset+offset+i);
            h->file_data[h->real_directory[index].offset+offset+i] = *(buff+i);
        }
    }
    fptr = fopen(h->f1, "wb");
    fwrite(h->file_data, sizeof(char), h->size_file, fptr);
    fclose(fptr);
    compute_hash_tree(helper);
    pthread_mutex_unlock(&lock);
    return 0;
}

ssize_t file_size(char * filename, void * helper) {
    // printf("file_size\n");

    struct Helper* h = (struct Helper*) helper;
    for(int i=0;i<h->size_directory;++i) {
        if(strcmp(h->directory_table[i].file_name, filename) == 0)
            return h->directory_table[i].length;
    }
    return -1;
}

void fletcher(uint8_t * buf, size_t length, uint8_t * output) {
    uint64_t a,b,c,d;
    uint32_t* buf2 = (uint32_t*) buf;
    a=b=c=d=0;
    for(int i=0;i<length/4;++i) {
        a = (a+*(buf2 + i))%((1UL<<32)-1);
        b = (b+a)%((1UL<<32)-1);
        c = (c+b)%((1UL<<32)-1);
        d = (d+c)%((1UL<<32)-1);
    }
    memcpy(output,&a,4);
    memcpy(output+4,&b,4);
    memcpy(output+8,&c,4);
    memcpy(output+12,&d,4);
    // output = hash_value;
}

void compute_hash_tree(void * helper) {
    // printf("compute_hash_tree\n");
    struct Helper* h = (struct Helper*) helper;
    h->hash_calculated = 1;
    for(int i=0;i<h->size_hash;++i)
        for(int j=0;j<16;++j)
            h->hash_data[i].byte[j] = 0;
    h->no_block = h->size_file/256;
    unsigned long int n = (unsigned long int) log2(h->no_block);
    h->no_node = ((1UL<<(n+1)) - 1);
    for(int i=0;i<h->no_block;++i) {
        fletcher((uint8_t*)(h->file_data+256*i), 256, (uint8_t*)(h->hash_data+(1UL<<n)-1+i));
    }
    for(int i=(1UL<<(n))-2;i>=0;--i) {
        fletcher((uint8_t*)(h->hash_data+2*i+1),32,(uint8_t*)(h->hash_data+i));
    }
    // for(int i=0;i<h->size_hash;++i)
    //     for(int j=0;j<16;++j)
    //         printf("%d ", h->hash_data[i].byte[j]);
    // printf("\n");
    write_data(helper);
    return;
}

void compute_hash_block(size_t block_offset, void * helper) {
    struct Helper* h = (struct Helper*) helper;
    if(h->hash_calculated == 0) {
        // for(int i=0;i<h->no_node;++i) {
        //     for(int j=0;j<16;++j)
        //         printf("%d ", (uint8_t)h->hash_data[i][j]);
        // }
        compute_hash_tree(helper);
    }
    unsigned long int index = block_offset + h->no_node - h->no_block;
    while(index>0) {
        if(index%2==0) {
            fletcher((uint8_t*)(h->hash_data+index-1),32,(uint8_t*)(h->hash_data+(index-2)/2));
            index = (index-2)>>1;
        } else {
            fletcher((uint8_t*)(h->hash_data+index),32,(uint8_t*)(h->hash_data+(index-1)/2));
            index = (index-1)>>1;
        }
    }
    write_data(helper);
    return;
}
