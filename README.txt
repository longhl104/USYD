-- How to use the FUSE testing guide --

This guide is intended to assist you with testing your FUSE implementation. Due to Ed limitations, we cannot automatically test your FUSE implementations before the deadline. Your computer where you run these tests may be different from the controlled testing environment.

Usage of this guide is no guarantee that you will receive a particular mark or pass our test cases.

-- File structure --

Directory "files" contains the file_data, directory_table and hash_data files you will be using. Files ending in ".before" are the starting templates for you to use. Files ending in ".after" are the expected ending versions, after you run your test, you need to compare "file_data", "directory_table" and "hash_data" against them.

Directory "mount_point" is empty for you to use as a mount point.

-- Usage --

Compile your myfuse.c . Place the resulting binary in this directory containing the README.

Open two terminals and navigate to the directory where this README is.

In one terminal, execute

./myfuse -d mount_point --files files/file_data files/directory_table files/hash_data

This runs your FUSE implementation of myfilesystem, based off the specified file data, directory table and hash data files, and makes it accessible at mount_point. The -d option shows you debugging information and also keeps the program in the foreground so you can see any output that your implementation prints.

In the other terminal, execute

gcc -o testme -Wall -Werror -pedantic -std=gnu11 testme.c

./testme

The testme program will run through a number of basic tests of your FUSE filesystem. Errors will be printed and the test program will terminate if this occurs. You have the source code available in testme.c, you are encouraged to review it to debug your errors and understand which system calls produce which FUSE callback functions. Please note that testme.c is only a representative sample of the tests that will be performed.

-- Don't Forget --

To run the ./myfuse command to mount your filesystem at mount_point.

Compare "file_data", "directory_table" and "hash_data" against the ".after" files. They should be equivalent.

Reset your "file_data", "directory_table" and "hash_data" files using the ".before" copies before you rerun the test, otherwise the test would be starting from a different starting point.
