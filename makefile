myfuse: myfuse.out
	echo "Fuse built!"

myfuse.out: myfuse.c myfilesystem.c myfilesystem.h
	gcc -o myfuse myfuse.c myfilesystem.c `pkg-config fuse --cflags --libs` -lm -lfuse

dummy: dummy.out
	echo "Some shit built!"

dummy.out: dummy.c
	gcc -Wall dummy.c -o dummy -lm

runtest: runtest.out
	echo "Build done!"

runtest.out: runtest.o myfilesystem.o
	gcc runtest.o myfilesystem.o -o runtest -lm

runtest.o: runtest.c myfilesystem.h
	gcc -c -Wall runtest.c -lm

myfilesystem.o: myfilesystem.c myfilesystem.h
	gcc -c -Wall myfilesystem.c -lm

clean:
	rm -f *.out *.o
