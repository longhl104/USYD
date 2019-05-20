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
    memset(result, 0, sizeof(struct stat));
    if (strcmp(name, "/") == 0) {
        result->st_mode = S_IFDIR;
        result->st_nlink = 2;
    } else {
        result->st_mode = S_IFREG;
        result->st_nlink = 1;
    }
    return 0;
}

int myfuse_readdir(const char * name, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi) {
    // MODIFY THIS FUNCTION
    if (strcmp(name, "/") == 0) {
    }
    // return 0;

    return 0;
}

int myfuse_unlink(const char * name) {
    return delete_file(name, &helper);
}

int myfuse_rename(const char * oldname, const char * newname) {
    return rename_file(oldname, newname, &helper);
}

int myfuse_truncate(const char *, off_t);
    // FILL OUT

int myfuse_open(const char *, struct fuse_file_info *);
    // FILL OUT

int myfuse_read(const char *, char *, size_t, off_t, struct fuse_file_info *);
    // FILL OUT

int myfuse_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);
    // FILL OUT

int myfuse_release(const char *, struct fuse_file_info *);
    // FILL OUT

void * myfuse_init(struct fuse_conn_info *);
    // FILL OUT

void myfuse_destroy(void *);
    // FILL OUT

int myfuse_create(const char *, mode_t, struct fuse_file_info *);
    // FILL OUT

struct fuse_operations operations = {
    .getattr = myfuse_getattr,
    .readdir = myfuse_readdir,
    .unlink = myfuse_unlink,
    // .rename =
    // .truncate =
    // .open =
    // .read =
    // .write =
    // .release =
    // .init =
    // .destroy =
    // .create =
};

int main(int argc, char * argv[]) {
    // MODIFY (OPTIONAL)
    if (argc >= 5) {
        if (strcmp(argv[argc-4], "--files") == 0) {
            file_data_file_name = argv[argc-3];
            directory_table_file_name = argv[argc-2];
            hash_data_file_name = argv[argc-1];
            argc -= 4;
            init_fs(file_data_file_name, directory_table_file_name, hash_data_file_name, 4);
        }
    }
    // After this point, you have access to file_data_file_name, directory_table_file_name and hash_data_file_name
    int ret = fuse_main(argc, argv, &operations, NULL);
    return ret;
}
