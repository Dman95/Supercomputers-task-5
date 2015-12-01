#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

typedef struct sparse_value
{
    long long index;
    long long value;
} sparse_value;

typedef struct sparse_vector
{
    long long size;
    sparse_value *values;
} sparse_vector;

sparse_vector *sv_new(long long size)
{
    sparse_vector *vector = calloc(1, sizeof(sparse_vector));
    vector->values = calloc(size, sizeof(sparse_value));
    return vector;
}

void sv_push_back(sparse_vector *vector, sparse_value *value)
{
    vector->values[vector->size++] = *value;
}

void sv_shrink_to_fit(sparse_vector *vector)
{
    realloc(vector->values, vector->size * sizeof(sparse_value));
}

void sv_delete(sparse_vector *vector)
{
    free(vector->values);
    free(vector);
}

typedef struct sparse_matrix
{
    long long row_count;
    long long column_count;
    sparse_vector **rows;
} sparse_matrix;

sparse_matrix sm_new(long long row_count, long long column_count)
{   
    sparse_matrix *matrix = calloc(1, sizeof(sparse_matrix));
    matrix->rows = calloc(row_count, sizeof(sparse_vector *));
    matrix->row_count = row_count;
    matrix->column_count = column_count;
    return matrix;
}

void sm_set_vector(sparse_matrix *matrix, int index, sparse_vector *vector)
{
    matrix->rows[index] = vector;
}

void sm_delete(sparse_matrix *matrix)
{
    for (long long i = 0; i < matrix->size; ++i) {
        sv_delete(matrix->rows[i]);
    }
    free(matrix);
}

void generate_sparse_vector(sparse_vector **vector, long long size, double non_zero_probability)
{
    *vector = sv_new(size);
    count = 0;
    for (long long i = 0; i < size; ++i) {
        double p = rand() / RAND_MAX;
        if (p < non_zero_probability) {
            unsigned short r = rand();
            sparse_value v;
            v.index = i;
            v.value = r;
            sv_push_back(vector, &v);
        }
    }
    sv_shrink_to_fit(vector);
}

void sv_write(sparse_vector *v, MPI_File *f)
{
    MPI_Status s;
    MPI_file_write(f, &v->size, 1, MPI_LONG_LONG, &s);
    MPI_File_write(f, v->values, v->size * sizeof(sparse_value), MPI_CHAR, &s);
}

sparse_vector *sv_read(MPI_file *f)
{
    MPI_Status s;
    long long size;
    MPI_File_read(f, &size, 1, MPI_LONG_LONG, &s);
    sparse_vector *vector = sv_new(size);
    MPI_File_read(f, vector->values, size * sizeof(sparse_value), MPI_CHAR, &s);
    vector->size = size;
    return vector;
}

void generate_and_save_sparse_matrix(char *filename, long long size, double non_zero_probability)
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f);
    MPI_File_write(f, &size, 1, MPI_LONG_LONG, &s);

    long long position = sizeof(long long);

    //omit section for offsets
    MPI_File_seek(f, size * sizeof(long long), MPI_SEEK_CUR);
    long long *offsets = calloc(size, sizeof(long long));
    position += size * sizeof(long long);

    for (long long i = 0; i < size; ++i) {
        sparse_vector *vector;
        generate_sparse_vector(&vector, size, non_zero_probability);
        offsets[i] = position;
        sv_write(vector, &f);
        position += vector->size;
        free(vector);
    }
    MPI_File_seek(f, sizeof(long long), MPI_SEEK_SET);
    MPI_File_write(f, offsets, size, MPI_LONG_LONG, &s);
    free(offsets);
    MPI_File_close(&f);
}

void load_sparse_matrix_part(char *filename, sparse_matrix **matrix, long long which, long long from_how_much, long long *row_count, long long *column_count) 
//e.g. which = 0, from_how_much = 4 should load 25% of rows
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
    long long size;
    MPI_File_read(f, &size, 1, MPI_LONG_LONG, &s);
    long long one_part_size_in_rows = size / from_how_much;

    long long *offsets = calloc(size, sizeof(long long));
    MPI_File_read(f, offsets, size, MPI_LONG_LONG, &s);

    long long start_offset = offsets[one_part_size_in_rows * which];
    MPI_File_seek(f, start_offset, MPI_SEEK_SET);

    long long remainder = size - one_part_size_in_rows * from_how_much;
    long long needed_to_read_rows_count = one_part_size_in_rows + ((which == from_how_much - 1) ? remainder : 0);

    *matrix = sm_new(needed_to_read_rows_count, size);
    for (int i = 0; i < needed_to_read_rows_count; ++i) {
        sparse_vector *vector = sv_read(f);
        sm_set_vector(*matrix, i, vector);
    }
    MPI_File_close(&f);
    *row_count = needed_to_read_rows_count;
    *column_count = size;
}

long long dot(sparse_vector *v1, long long *v2)
{
    long long result = 0;
    for (long long i = 0; i < v1->size; ++i) {
        result += v1->values[i].value * v2[v1->values[i].index];
    }
    return result;
}

void multiply(sparse_matrix *matrix, long long *vector, long long *result)
{
    for (long long i = 0; i < matrix->row_count; ++i) {
        result[i] = dot(matrix->rows[i], vector);
    }
}

void print_sparse_matrix(sparse_matrix *matrix)
{
    printf("Matrix %lld x %lld\n", matrix->row_count, matrix->column_count);
    for (long long i = 0; i < matrix->row_count; ++i) {
        printf("[");
        for (long long j = 0; j < matrix->rows[i]->size; ++j) {
            printf("(%lld %lld) ", matrix->rows[i]->values[j].index, matrix->rows[i]->values[j].value);
        }
        printf("]\n");
    }
}

