program: main.o lib.o
	mpicc main.o lib.o -o main
main.o: main.c
	mpicc main.c -c --std=c99 -o main.o
main.c: lib.h
lib.o: lib.c
	mpicc lib.c -c --std=c99 -o lib.o
lib.c: lib.h

clean:
	rm main.o lib.o main
