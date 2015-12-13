program: main
main: main.o lib/matrix_lib.o lib/sparse_lib.o
	mpixlc_r -qsmp=omp -O3 -qarch=450 -qtune=450 main.o lib/matrix_lib.o -o main
main.o: main.c
	mpixlc_r -qsmp=omp -O3 -qarch=450 -qtune=450 main.c -c -o main.o
main.c: lib/matrix_lib.h
lib/matrix_lib.o: lib/matrix_lib.c
	mpixlc_r -qsmp=omp -O3 -qarch=450 -qtune=450 lib/matrix_lib.c -c -o lib/matrix_lib.o
lib/matrix_lib.c: lib/matrix_lib.h
clean:
	rm main.o lib/matrix_lib.o main
