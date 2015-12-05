#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "lib/matrix_lib.h"
#include <string.h>
#include "lib/sparse_lib.h"

int main(int argc, char **argv)
{
    char vectorname[100];
    char matrixname[100];
    char resultname[100];
    
    MPI_Init(&argc, &argv);
    double start = -MPI_Wtime();

    vector *v;
    if (!strcmp(argv[3], "--sparse")) {
        sprintf(vectorname, "datasparse/v%s", argv[1]);
        sprintf(matrixname, "datasparse/m%s", argv[1]);
        sprintf(resultname, "resultssparse/mul%snproc%s", argv[1], argv[2]);
        v = mpi_sparse_multiply(matrixname, vectorname);
    } else {
        sprintf(vectorname, "data/v%s", argv[1]);
        sprintf(matrixname, "data/m%s", argv[1]);
        sprintf(resultname, "results/mul%snproc%s", argv[1], argv[2]);
        v = mpi_multiply(matrixname, vectorname);
    }
    
    if (v) {
        vector_save(v, resultname);
        vector_delete(v);
        double time_elapsed = start + MPI_Wtime();
        printf("Time: %f\n", time_elapsed);
    }

    MPI_Finalize();
    return 0;
}
