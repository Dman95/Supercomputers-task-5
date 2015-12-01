#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "lib.h"

/* void generate_vector(long long **vector, long long size);
 * void generate_matrix(long long **matrix, long long size);
 * void generate_and_save_matrix(char *filename, long long size);
 * long long load_vector(char *filename, long long **vector);
 * void save_vector(char *filename, long long *vector, long long size);
 * long long load_matrix(char *filename, long long **matrix);
 * void load_matrix_part(char *filename, long long **matrix, long long which, long long from_how_much, 
 *                       long long *row_count, long long *column_count); 
 * //e.g. which = 0, from_how_much = 4 should load 25% of rows
 * void save_matrix(char *filename, long long *matrix, long long size);
 * long long dot(long long *v1, long long *v2, long long size);
 * void multiply(long long *matrix, long long *vector, long long row_count, long long column_count, long long *result);
 * void print_matrix(long long *matrix, long long row_count, long long column_count);
 */

int main(int argc, char **argv)
{
    char vectorname[100];
    char matrixname[100];
    char resultname[100];
    char debugname[100];
    sprintf(vectorname, "v%s", argv[1]);
    sprintf(matrixname, "m%s", argv[1]);
    sprintf(resultname, "mul%snproc%s", argv[1], argv[2]);

    int size, myrank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    double start = -MPI_Wtime();

    long long *matrix, *vector;
    long long row_count, column_count;
    load_matrix_part(matrixname, &matrix, myrank, size, &row_count, &column_count);
    
    load_vector(vectorname, &vector);
    long long *result = calloc(row_count, sizeof(long long));
    multiply(matrix, vector, row_count, column_count, result);
    
    long long *final_result = 0;
    int *recvcounts = 0;
    int *displs = 0;
    if (!myrank) {
        final_result = calloc(column_count, sizeof(long long));
        recvcounts = calloc(size, sizeof(int));
        displs = calloc(size, sizeof(int));
        int count = column_count / size;
        int displ = 0;
        for (int i = 0; i < size - 1; ++i) {
            recvcounts[i] = count;
            displs[i] = displ;
            displ += count;
        }
        recvcounts[size - 1] = column_count - displ;
        displs[size - 1] = displ;
    }
    MPI_Gatherv(result, row_count, MPI_LONG_LONG, final_result, recvcounts, displs, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    
    if (!myrank) {
        save_vector(resultname, final_result, column_count);
        free(final_result);
        free(recvcounts);
        free(displs);
        double time_elapsed = start + MPI_Wtime();
        printf("Time: %f\n", time_elapsed);
    }
    free(matrix);
    free(vector);
    free(result);
    MPI_Finalize();
    return 0;
}
