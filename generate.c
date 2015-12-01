#include "lib.h"
#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int size;
    char vectorname[100];
    char matrixname[100];
    sprintf(vectorname, "v%s", argv[1]);
    sprintf(matrixname, "m%s", argv[1]);
    sscanf(argv[1], "%d", &size);
    printf("%d\n", size);
    long long *vector;
    generate_vector(&vector, size);
    save_vector(vectorname, vector, size);
    free(vector);
    generate_and_save_matrix(matrixname, size); 
    MPI_Finalize();
    return 0;
}
