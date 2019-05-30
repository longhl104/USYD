/* Do not change! */
#define FUSE_USE_VERSION 29
#define _FILE_OFFSET_BITS 64
/******************/
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fuse.h>
#include <errno.h>
#include "myfilesystem.h"
#include <dirent.h>

char * file_data_file_name = NULL;
char * directory_table_file_name = NULL;
char * hash_data_file_name = NULL;

int myfuse_getattr(const char * name, struct stat * result) {
    // MODIFY THIS FUNCTION
    lstat(name, result);
    if (strcmp(name, "/") == 0) {
        result->st_mode = S_IFDIR | 0755;
        result->st_nlink = 2;
        return 0;
    } else {
        for(int i=0; i<helper.no_directory; ++i)
            if(strcmp(helper.real_directory[i].file_name, name + 1) == 0) {
                result->st_mode = S_IFREG | 0444;
                result->st_nlink = 1;
                result->st_size = file_size(name + 1, &helper);
                return 0;
            }
        return -ENOENT;
    }
}

void print_fuse(struct fuse_file_info * fi) {
    // printf("flags: %d\n", fi->flags);
    // printf("writepage: %ld\n", fi->writepage);
    // printf("direct_io: %ld\n", fi->direct_io);
    // printf("keep_cache: %ld\n", fi->keep_cache);
    // printf("flush: %d\n", fi->flush);
    // printf("nonseekable: %d\n", fi->nonseekable);
    // printf("padding: %d\n", fi->padding);
    // printf("fh: %d\n", fi->fh);
    // printf("lock_owner: %d\n", fi->lock_owner);
}

int myfuse_readdir(const char * name, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi) {
    // MODIFY THIS FUNCTION
    if (strcmp(name, "/") == 0) {
        for(int i=0; i<helper.no_directory; ++i) {
            filler(buf, helper.real_directory[i].file_name, NULL, 0);
        }
        return 0;
    } else return -ENOENT;
}

int myfuse_unlink(const char * path) {
    int res;
    res = delete_file(path + 1, &helper);
    if(res == 1)
        return -ENOENT;
    unlink(path);
    return 0;
}

int myfuse_rename(const char * oldname, const char * newname) {
    int res = rename_file(oldname + 1, newname + 1, &helper);
    if(res == 1)
        return -ENOENT;
    rename(oldname, newname);
    // for(int i=0;i<helper.no_directory;++i)
    //     printf("%s\n", helper.real_directory[i].file_name);
    return 0;
}

int myfuse_truncate(const char * path, off_t length, struct fuse_file_info *fi) {
    int res;
    res = resize_file(path + 1, length, &helper);
    if(res == 1)
        return -ENOENT;
    if(res == 2)
        return -EFBIG;
    res = ftruncate(fi->fh, length);
    return 0;
}

int myfuse_open(const char * path, struct fuse_file_info * fi) {
    print_fuse(fi);
    int res = 1;
    for(int i=0;i<helper.no_directory;++i) {
        if(strcmp(path+1, helper.real_directory[i].file_name) == 0)
            res = 0;
    }
    if (res == 1) {
        fi->fh = -1;
        return -ENOENT;
    }

    // if ((fi->flags & O_ACCMODE) != O_RDONLY)
    //     return -EACCES;

    fi->fh = open(path, fi->flags);
    return 0;
}

int myfuse_read(const char * path, char * buf, size_t size , off_t offset, struct fuse_file_info * fi) {
    print_fuse(fi);
    int res;
    int len = file_size(path+1, &helper);
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        res = read_file(path + 1, offset, size, buf, &helper);
    } else
        return 0;

    if (res == 1)
        return -ENOENT;
    if (res == 2)
        return -EPERM;

    return size;
}

int myfuse_write(const char *path , const char *buf, size_t size, off_t offset, struct fuse_file_info * fi) {
    print_fuse(fi);
    fi->fh = 3;
    int res;
    res = write_file(path + 1, offset, size, buf, &helper);
    if (res == 1)
        return -ENOENT;
    if (res == 2)
        return -EPERM;
    if (res == 3)
        return -EFBIG;
    return size;
}

int myfuse_release(const char * path, struct fuse_file_info * fi) {
    (void) path;
    close(fi->fh);
    return 0;
}

void * myfuse_init(struct fuse_conn_info * conn) {
    (void) conn;
    init_fs(file_data_file_name, directory_table_file_name, hash_data_file_name, 2);
    for(int i=0;i<helper.no_directory;++i) {
        int fo = open(helper.real_directory[i].file_name, O_CREAT | O_RDWR | O_EXCL, S_IRWXU);
        write(fo, helper.file_data + helper.real_directory[i].offset, helper.real_directory[i].length);
        close(fo);
    }
    return NULL;
}

void myfuse_destroy(void * dummy) {
    close_fs(&helper);
    return NULL;
}

int myfuse_create(const char * path, mode_t mode, struct fuse_file_info * fi) {
    print_fuse(fi);
    int res;
    res = create_file(path + 1, 0, &helper);
    if (res == 1)
        return -ENOENT;
    if (res == 2)
        return -EFBIG;
    fi->fh = open(path, fi->flags, mode);
    return 0;
}

struct fuse_operations operations = {
    .getattr = myfuse_getattr,
    .readdir = myfuse_readdir,
    .unlink = myfuse_unlink,
    .rename = myfuse_rename,
    .truncate = myfuse_truncate,
    .open = myfuse_open,
    .read = myfuse_read,
    .write = myfuse_write,
    .release = myfuse_release,
    .init = myfuse_init,
    .destroy = myfuse_destroy,
    .create = myfuse_create
};

// char* get_path(char* file) {
//     char* res = malloc(strlen(file) + 2);
//     res[0] = '/';
//     strcpy(res+1, file);
//     res[strlen(file) + 1] = '\0';
//     return res;
// }
int main(int argc, char * argv[]) {
    // MODIFY (OPTIONAL)
    if (argc >= 5) {
        if (strcmp(argv[argc-4], "--files") == 0) {
            file_data_file_name = argv[argc-3];
            directory_table_file_name = argv[argc-2];
            hash_data_file_name = argv[argc-1];
            argc -= 4;
            // printf("%s %s %s\n", file_data_file_name, directory_table_file_name, hash_data_file_name);
            // printf("%d\n", helper.no_directory);
        }
    }
    // After this point, you have access to file_data_file_name, directory_table_file_name and hash_data_file_name
    int ret = fuse_main(argc, argv, &operations, NULL);
    return ret;
}
