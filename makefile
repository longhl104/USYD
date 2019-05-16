dummy: dummy.out
	echo "Some shit built!"

dummy.out: dummy.c
	gcc -Wall dummy.c -o dummy

runtest: runtest.out
	echo "Build done!"

runtest.out: runtest.o myfilesystem.o
	gcc runtest.o myfilesystem.o -o runtest

runtest.o: runtest.c myfilesystem.h
	gcc -c -Wall runtest.c

myfilesystem.o: myfilesystem.c myfilesystem.h
	gcc -c -Wall myfilesystem.c

clean:
	rm -f *.out *.o
