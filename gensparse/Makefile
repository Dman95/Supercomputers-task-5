program: generate_sparse
generate_sparse: generate_sparse.o ../lib/matrix_lib.o ../lib/sparse_lib.o
	mpicc generate_sparse.o ../lib/matrix_lib.o ../lib/sparse_lib.o -lm -o generate_sparse
generate_sparse.o: generate_sparse.c
	mpicc generate_sparse.c -c --std=c99 -o generate_sparse.o
generate_sparse.c: ../lib/matrix_lib.h ../lib/sparse_lib.h
../lib/matrix_lib.o: ../lib/matrix_lib.c
	mpicc ../lib/matrix_lib.c -c --std=c99 -o ../lib/matrix_lib.o
../lib/matrix_lib.c: ../lib/matrix_lib.h
../lib/sparse_lib.o: ../lib/sparse_lib.c
	mpicc ../lib/sparse_lib.c -c --std=c99 -o ../lib/sparse_lib.o
../lib/sparse_lib.c: ../lib/sparse_lib.h
../lib/sparse_lib.h: ../lib/matrix_lib.h

clean:
	rm generate_sparse.o ../lib/matrix_lib.o ../lib/sparse_lib.o generate_sparse
