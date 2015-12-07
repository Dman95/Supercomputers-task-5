#include "../lib/matrix_lib.h"
#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"
#include <time.h>

//generate square matrix and vector by size
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int A_row_count, A_column_count, B_row_count, B_column_count;
    char A_matrixname[100];
    char B_matrixname[100];
    sprintf(A_matrixname, "../data/A%sx%s", argv[1], argv[2]);
    sprintf(B_matrixname, "../data/B%sx%s", argv[3], argv[4]);
    sscanf(argv[1], "%d", &A_row_count);
    sscanf(argv[2], "%d", &A_column_count);
    sscanf(argv[3], "%d", &B_row_count);
    sscanf(argv[4], "%d", &B_column_count);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    double start = -MPI_Wtime();

    srand(7 * time(NULL) + 1046527 * rank);
    //generate A matrix
    mpi_matrix_gen_and_save(A_row_count, A_column_count, A_matrixname);

    //generate B matrix
    mpi_matrix_gen_and_save(B_row_count, B_column_count, B_matrixname);
    
    printf("Time: %f\n", start + MPI_Wtime());

    MPI_Finalize();
    return 0;
}
