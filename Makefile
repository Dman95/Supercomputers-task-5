program: main
main: main.o lib/matrix_lib.o lib/sparse_lib.o
	mpicc main.o lib/matrix_lib.o lib/sparse_lib.o -lm -o main
main.o: main.c
	mpicc main.c -c --std=c99 -o main.o
main.c: lib/matrix_lib.h
lib/matrix_lib.o: lib/matrix_lib.c
	mpicc lib/matrix_lib.c -c --std=c99 -o lib/matrix_lib.o
lib/matrix_lib.c: lib/matrix_lib.h
lib/sparse_lib.o: lib/sparse_lib.c
	mpicc lib/sparse_lib.c -c --std=c99 -o lib/sparse_lib.o
lib/sparse_lib.c: lib/sparse_lib.h
lib/sparse_lib.h: lib/matrix_lib.h
clean:
	rm main.o lib/matrix_lib.o lib/sparse_lib.o main
