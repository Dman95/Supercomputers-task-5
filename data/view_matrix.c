#include "../lib/matrix_lib.h"

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    for (int i = 1; i < argc; ++i) {
        matrix *m = matrix_load(argv[i]);
        matrix_print(m);
        matrix_delete(m);
    }

    MPI_Finalize();
}
