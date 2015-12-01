#include "../lib/matrix_lib.h"

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    matrix *m = matrix_load(argv[1]);
    matrix_print(m);
    matrix_delete(m);

    vector *v = vector_load(argv[2]);
    vector_print(v);
    vector_delete(v);

    MPI_Finalize();
}
