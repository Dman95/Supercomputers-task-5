#include "../lib/sparse_lib.h"
#include "../lib/matrix_lib.h"
#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"
#include <time.h>

//generate square sparse matrix and vector by size
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int size, coef;
    char vectorname[100];
    char matrixname[100];
    sprintf(vectorname, "../datasparse/v%s", argv[1]);
    sprintf(matrixname, "../datasparse/m%s", argv[1]);
    sscanf(argv[1], "%d", &size);
    sscanf(argv[2], "%d", &coef);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    srand(7 * time(NULL) + 1046527 * rank);

    double start = -MPI_Wtime();
    if (!rank) {
        //generate vector
        vector *v = vector_gen(size);
        vector_save(v, vectorname);
        vector_delete(v);
    }
    
    //generate sparse matrix
    mpi_sparse_matrix_gen_and_save(size, size, matrixname, 1.0 * coef / size);
    
    if (!rank) {
        printf("Time: %f\n", MPI_Wtime() + start);
    }

    MPI_Finalize();
    return 0;
}
