#ifndef SPARSE_LIB_H
#define SPARSE_LIB_H

#include "mpi.h"
#include "matrix_lib.h"

//random
double uniform_random();
long long geom_random(double non_zero_probability, long long max);


//sparse vector operations
typedef struct sparse_value
{
    long long index;
    double value;
} sparse_value;

typedef struct sparse_vector
{
    long long non_zero_count;
    long long size;
    sparse_value *values;
} sparse_vector;

sparse_vector *sparse_vector_new(long long size);
sparse_vector *sparse_vector_gen(long long size, double non_zero_probability);
sparse_vector *sparse_vector_load(char *filename);
sparse_vector *sparse_vector_load_fp(MPI_File *f);
long long sparse_vector_serialized_size(sparse_vector *v);
void sparse_vector_save(sparse_vector *v, char *filename);
void sparse_vector_save_fp(sparse_vector *v, MPI_File *f);
void sparse_vector_push_back(sparse_vector *v, long long index, double value);
void sparse_vector_shrink_to_fit(sparse_vector *v);
void sparse_vector_print(sparse_vector *v);
char *sparse_vector_serialize(sparse_vector *v);
sparse_vector *sparse_vector_deserialize(char *serialized);
void sparse_vector_delete(sparse_vector *v);


//matrix operations
typedef struct sparse_matrix
{
    long long row_count;
    long long column_count;
    sparse_vector **rows;
} sparse_matrix;

sparse_matrix *sparse_matrix_new(long long row_count, long long column_count);
sparse_matrix *sparse_matrix_new_without_vector_init(long long row_count, long long column_count);
sparse_matrix *sparse_matrix_gen(long long row_count, long long column_count, double non_zero_probability);
sparse_matrix *sparse_matrix_load(char *filename);
MPI_File sparse_matrix_get_file_started_from_part(char *filename, long long which, long long from_how_much, long long *needed_to_read_row_count, long long *column_count);
sparse_matrix *sparse_matrix_load_part(char *filename, long long which, long long from_how_much);
void sparse_matrix_save(sparse_matrix *m, char *filename);
void sparse_matrix_gen_and_save(long long row_count, long long column_count, char *filename, double non_zero_probability);
void mpi_sparse_matrix_gen_and_save(long long row_count, long long column_count, char *filename, double non_zero_probability);
void sparse_matrix_print(sparse_matrix *m);
void sparse_matrix_delete(sparse_matrix *m);


//multiplication operations
double sparse_dot(sparse_vector *v1, vector *v2);
vector *sparse_multiply(sparse_matrix *m, vector *v);
vector *mpi_sparse_multiply(char *matrixname, char *vectorname);

#endif

