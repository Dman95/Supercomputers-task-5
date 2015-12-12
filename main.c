#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "lib/matrix_lib.h"
#include <string.h>
#include "lib/sparse_lib.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double count_radius(long long m)
{
    return 1 - pow(M_PI / (2 * (m + 1)), 2);
}

double count_w(long long n, double prev_w, double radius)
{
    if (n == 0) {
        return 0;
    }
    if (n == 1) {
        return 1 / (1 - radius * radius / 2);
    }
    return 1 / (1 - radius * radius * prev_w / 4);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    double start = -MPI_Wtime();
    
    //read cl parameters
    long long m;
    double precision;
    if (argc < 3) {
        precision = 0.01;
        if (argc < 2) {
            m = 512;
        } else {
            sscanf(argv[1], "%lld", &m);
        }
    } else {
        sscanf(argv[2], "%lf", &precision);
    }

    int myrank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //init
    long long row_count = count_part(myrank, size, m);
    long long column_count = m;
    matrix *u = matrix_new(row_count + 2, column_count + 2);
    matrix *us = matrix_new(row_count + 2, column_count + 2);
    double dx, dy;
    dx = dy = 1.0 / (m + 1);
    if (myrank == 0) {
        for (long long j = 0; j < column_count + 2; ++j) {
            u->rows[0]->values[j] = us->rows[0]->values[j] = sin(M_PI * dx * j);
        }
    }
    if (myrank == size - 1) {
        for (long long j = 0; j < column_count + 2; ++j) {
            u->rows[row_count - 1]->values[j] = us->rows[row_count - 1]->values[j] = sin(M_PI * dx * j) * exp(-dx * j);
        }
    }
    
    //count
    double radius = count_radius(m);
    double w = 0;
    for (long long n = 0; n < 10; ++n) {
        for (long long i = 1; i < us->row_count - 1; ++i) {
            for (long long j = 1; j < us->column_count - 1; ++j) {
                us->rows[i]->values[j] = (u->rows[i - 1]->values[j] + u->rows[i + 1]->values[j] +
                                         u->rows[i]->values[j - 1] + u->rows[i]->values[j + 1]) / 4;
            }
        }
        w = count_w(n, w, radius);
        for (long long i = 1; i < us->row_count - 1; ++i) {
            for (long long j = 1; j < us->column_count - 1; ++j) {
                u->rows[i]->values[j] = w * us->rows[i]->values[j] + (1 - w) * u->rows[i]->values[j];
            }
        }
        
        //exchanges
        MPI_Status s;
        //exchange with top
        if (myrank != 0) {
            MPI_Sendrecv(u->rows[1]->values, u->rows[1]->size, MPI_DOUBLE, myrank - 1, n, 
                         u->rows[0]->values, u->rows[0]->size, MPI_DOUBLE, myrank - 1, n, 
                         MPI_COMM_WORLD, &s);
        }
        //exchange with bottom
        if (myrank != size - 1) {
            MPI_Sendrecv(u->rows[u->row_count - 2]->values, u->rows[u->row_count - 2]->size, MPI_DOUBLE, myrank + 1, n,
                         u->rows[u->row_count - 1]->values, u->rows[u->row_count - 1]->size, MPI_DOUBLE, myrank + 1, n, 
                         MPI_COMM_WORLD, &s);
        
        }
    }
    
    double time_elapsed = start + MPI_Wtime();
    printf("Time: %f\n", time_elapsed);

    MPI_Finalize();
    return 0;
}
