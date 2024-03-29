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
        return 1.0 / (1 - radius * radius / 2);
    }
    return 1.0 / (1 - radius * radius * prev_w / 4);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    double start = -MPI_Wtime();
    
    //read cl parameters
    long long m;
    double precision;
    if (argc >= 3) {
        sscanf(argv[2], "%lf", &precision);
    } else {
        precision = 0.01;
    }
    if (argc >= 2) {
        sscanf(argv[1], "%lld", &m);
    } else {
        m = 512;
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
        #pragma omp parallel for
        for (long long j = 0; j < column_count + 2; ++j) {
            u->rows[0]->values[j] = us->rows[0]->values[j] = sin(M_PI * dx * j);
        }
    }
    if (myrank == size - 1) {
        #pragma omp parallel for
        for (long long j = 0; j < column_count + 2; ++j) {
            u->rows[u->row_count - 1]->values[j] = us->rows[us->row_count - 1]->values[j] = sin(M_PI * dx * j) * exp(-dx * j);
        }
    }
    
    //count
    double radius = count_radius(m);
    double w = 0;
    for (long long n = 1; ; ++n) {
        #pragma omp parallel for
        for (long long ij = 0; ij < (us->row_count - 2) * (us->column_count - 2); ++ij) {
            long long i = 1 + ij / (us->column_count - 2);
            long long j = 1 + ij % (us->column_count - 2);
            us->rows[i]->values[j] = (u->rows[i - 1]->values[j] + u->rows[i + 1]->values[j] +
                                      u->rows[i]->values[j - 1] + u->rows[i]->values[j + 1]) / 4;
        }
        w = count_w(n, w, radius);
        double cursum = 0;
        #pragma omp parallel for reduction( + : cursum )
        for (long long ij = 0; ij < (us->row_count - 2) * (us->column_count - 2); ++ij) {
            long long i = 1 + ij / (us->column_count - 2);
            long long j = 1 + ij % (us->column_count - 2);
            double prev = u->rows[i]->values[j];
            double newval = w * us->rows[i]->values[j] + (1 - w) * u->rows[i]->values[j];
            if (newval > prev) {
                u->rows[i]->values[j] = newval;
            } else {
                newval = prev;
            } 
            double diff = newval - prev;
            cursum += diff * diff;
        }
        double allsum = 0;
        MPI_Allreduce(&cursum, &allsum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        allsum = sqrt(allsum);

        if (!myrank && n % 1000 == 0) {
            printf("N: %lld maxsum: %f pr: %f\n", n, allsum, precision);
        }
        if (allsum <= precision) {
            break;
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

    char *resultname = argv[3];
    row_count += 2;
    column_count += 2;
    if (!myrank) {
        //write size
        MPI_File f;
        MPI_Status s;
        MPI_File_open(MPI_COMM_SELF, resultname, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f);
        MPI_File_write(f, &column_count, 1, MPI_LONG_LONG, &s);
        MPI_File_write(f, &column_count, 1, MPI_LONG_LONG, &s);
        MPI_File_close(&f);
    }
    long long offset = 2 * sizeof(long long);
    if (myrank == 0) {
        offset += 0;
    } else {
        offset += (1 + (row_count - 2) * myrank) * (sizeof(long long) + column_count * sizeof(double));
    }
    MPI_File resultfile = get_file_with_offset_for_write(resultname, offset);
    for (long long i = ((myrank != 0) ? 1 : 0); i < u->row_count - ((myrank != size - 1) ? 1 : 0); ++i) {
        vector_save_fp(u->rows[i], &resultfile);
    }
    MPI_File_close(&resultfile);

    matrix_delete(u);
    matrix_delete(us);
    
    double time_elapsed = start + MPI_Wtime();
    printf("Time: %f\n", time_elapsed);

    MPI_Finalize();
    return 0;
}
