#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

int main(void) {

    assert(chdir("mount_point") == 0);

    DIR * root = opendir(".");
    if (root == NULL ) {
        perror("opendir unexpectedly failed");
        return 1;
    }

    errno = 0;
    struct dirent * entry = readdir(root);
    if (entry == NULL) {
        if (errno == 0) {
            fputs("did not find expected directory entry\n", stderr);
            return 1;
        } else {
            perror("readdir unexpectedly failed");
            return 1;
        }
    }
    if (strcmp(entry->d_name, "existing_file") != 0) {
        fputs("incorrect directory entry\n", stderr);
        return 1;
    }
    errno = 0;
    entry = readdir(root);
    if (entry != NULL) {
        fputs("did not expect extra directory entries\n", stderr);
        return 1;
    } else if (errno != 0) {
        perror("readdir unexpectedly failed");
        return 1;
    }

    int ret = closedir(root);
    if (ret != 0) {
        perror("closedir unexpectedly failed");
        return 1;
    }

    ret = unlink("existing_file");
    if (ret != 0) {
        perror("unlink unexpectedly failed");
        return 1;
    }

    ret = open("dreamycookies", O_CREAT | O_EXCL | O_RDWR, S_IRWXU);
    if (ret == -1) {
        perror("open/creat unexpectedly failed");
        return 1;
    }

    int ret2 = write(ret, "cubecubecube", 12);
    if (ret2 != 12) {
        perror("write unexpectedly failed");
        return 1;
    }

    off_t sk = lseek(ret, 3, SEEK_SET);
    if (sk != 3) {
        perror("lseek unexpectedly failed");
        return 1;
    }

    char expected[5] = "ecube";
    char actual[5];
    ret2 = read(ret, actual, 5);
    if (ret2 != 5) {
        perror("read unexpectedly failed");
        return 1;
    }
    if (memcmp(expected, actual, 5) != 0) {
        fputs("read data was not correct\n", stderr);
        return 1;
    }

    ret2 = close(ret);
    if (ret2 != 0) {
        perror("close unexpectedly failed");
        return 1;
    }

    struct stat result;
    ret2 = stat("dreamycookies", &result);
    if (ret2 != 0) {
        perror("stat unexpectedly failed");
        return 1;
    }
    if (!S_ISREG(result.st_mode)) {
        fputs("file is not a regular file as expected on stat\n", stderr);
        return 1;
    }
    if (result.st_size != 12) {
        fputs("stat on file did not return expected file size\n", stderr);
        return 1;
    }

    ret2 = rename("dreamycookies", "synaptophysin");
    if (ret2 != 0) {
        perror("rename unexpectedly failed");
        return 1;
    }

    ret2 = truncate("synaptophysin", 6);
    if (ret2 != 0) {
        perror("truncate unexpectedly failed");
        return 1;
    }

    fputs("Completed OK\n", stderr);
    return 0;
}
