#include <stdio.h>

#define TEST(x) test(x, #x)
#include "myfilesystem.h"

/* You are free to modify any part of this file. The only requirement is that when it is run, all your tests are automatically executed */

/* Some example unit test functions */
int success() {
    return 0;
}

int failure() {
    return 1;
}

int no_operation() {
    void * helper = init_fs("file_data", "directory_table", "hash_data", 1); // Remember you need to provide your own test files and also check their contents as part of testing
    close_fs(helper);
    return 0;
}

int test_create_file() {
    void * helper = init_fs("file_data", "02_directory_table", "hash_data", 1); // Remember you need to provide your own test files and also check their contents as part of testing
    int res = create_file("document.txt",1,helper);
    res=create_file("config",3,helper);
    close_fs(helper);
    return res;
    return 0;
}

int test_repack() {
    return 0;
}

int test_resize() {
    void * helper = init_fs("05_file_data", "05_directory_table", "05_hash_data", 4);
    int ret = resize_file("assignment.pdf", 1025, helper);
    // ret = resize_file("assignment.pdf", 1024, helper);
    close_fs(helper);
    return ret;
}

int test_read_file() {
    void * helper = init_fs("10_file_data", "10_directory_table", "10_hash_data", 4);
    char buf[11];
    int ret = read_file("file1.txt", 5, 50, buf, helper);
    // ret = read_file("file1.txt", 5, 10, buf, helper);
    close_fs(helper);
    printf("%s\n", buf);
    return ret;
}
/****************************/

/* Helper function */
void test(int (*test_function) (), char * function_name) {
    int ret = test_function();
    if (ret == 0) {
        printf("Passed %s\n", function_name);
    } else {
        printf("Failed %s returned %d\n", function_name, ret);
    }
}
/************************/

int main(int argc, char * argv[]) {

    // You can use the TEST macro as TEST(x) to run a test function named "x"
    TEST(success);
    TEST(failure);
    TEST(no_operation);

    // Add more tests here
    // TEST(test_create_file);
    // TEST(test_resize);
    TEST(test_read_file);

    return 0;
}
