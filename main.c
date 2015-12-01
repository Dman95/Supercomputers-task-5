#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "lib/matrix_lib.h"

int main(int argc, char **argv)
{
    char vectorname[100];
    char matrixname[100];
    char resultname[100];
    sprintf(vectorname, "data/v%s", argv[1]);
    sprintf(matrixname, "data/m%s", argv[1]);
    sprintf(resultname, "results/mul%snproc%s", argv[1], argv[2]);

    int size, myrank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    double start = -MPI_Wtime();

    matrix *m = matrix_load_part(matrixname, myrank, size);
    vector *v = vector_load(vectorname);
    vector *result = multiply(m, v);

    vector *final_result = 0;
    int *recvcounts = 0;
    int *displs = 0;
    if (!myrank) {
        final_result = vector_new(v->size);
        recvcounts = calloc(size, sizeof(int));
        displs = calloc(size, sizeof(int));
        int count = v->size / size;
        int displ = 0;
        for (int i = 0; i < size - 1; ++i) {
            recvcounts[i] = count;
            displs[i] = displ;
            displ += count;
        }
        recvcounts[size - 1] = v->size - displ;
        displs[size - 1] = displ;
    }
    MPI_Gatherv(result->values, result->size, MPI_LONG_LONG, final_result->values, recvcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    if (!myrank) {
        vector_save(final_result, resultname);
        vector_delete(final_result);
        free(recvcounts);
        free(displs);
        double time_elapsed = start + MPI_Wtime();
        printf("Time: %f\n", time_elapsed);
    }
    
    matrix_delete(m);
    vector_delete(v);
    vector_delete(result);

    MPI_Finalize();
    return 0;
}
