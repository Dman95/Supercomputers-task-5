program: generate.o ../lib/matrix_lib.o
	mpicc generate.o ../lib/matrix_lib.o -o generate
generate.o: generate.c
	mpicc generate.c -c --std=c99 -o generate.o
generate.c: ../lib/matrix_lib.h
../lib/matrix_lib.o: ../lib/matrix_lib.c
	mpicc ../lib/matrix_lib.c -c --std=c99 -o ../lib/matrix_lib.o
../lib/matrix_lib.c: ../lib/matrix_lib.h

clean:
	rm generate.o ../lib/matrix_lib.o generate
