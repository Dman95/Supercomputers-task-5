program: view_matrix
view_matrix: view_matrix.o ../lib/matrix_lib.o
	mpicc view_matrix.o ../lib/matrix_lib.o -lm -o view_matrix
view_matrix.o: view_matrix.c
	mpicc view_matrix.c -c --std=c99 -o view_matrix.o
view_matrix.c: ../lib/matrix_lib.h
../lib/matrix_lib.o: ../lib/matrix_lib.c
	mpicc ../lib/matrix_lib.c -c --std=c99 -o ../lib/matrix_lib.o
../lib/matrix_lib.c: ../lib/matrix_lib.h

clean:
	rm view_matrix.o ../lib/matrix_lib.o view_matrix
