program: main.o lib/matrix_lib.o
	mpicc main.o lib/matrix_lib.o -o main
main.o: main.c
	mpicc main.c -c --std=c99 -o main.o
main.c: lib/matrix_lib.h
lib/matrix_lib.o: lib/matrix_lib.c
	mpicc lib/matrix_lib.c -c --std=c99 -o lib/matrix_lib.o
lib/matrix_lib.c: lib/matrix_lib.h

clean:
	rm main.o lib/matrix_lib.o main
