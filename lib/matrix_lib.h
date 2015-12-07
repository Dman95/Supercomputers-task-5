#ifndef MATRIX_LIB_H
#define MATRIX_LIB_H

#include "mpi.h"

//vector operations
typedef struct vector
{
    long long size;
    double *values;
} vector;

vector *vector_new(long long size);
vector *vector_gen(long long size);
vector *vector_load(char *filename);
vector *vector_load_fp(MPI_File *f);
void vector_save(vector *v, char *filename);
void vector_save_fp(vector *v, MPI_File *f);
void vector_print(vector *v);
void vector_delete(vector *v);


//matrix operations
typedef struct matrix
{
    long long row_count;
    long long column_count;
    vector **rows;
} matrix;

matrix *matrix_new(long long row_count, long long column_count);
matrix *matrix_new_without_vector_init(long long row_count, long long column_count);
matrix *matrix_gen(long long row_count, long long column_count);
matrix *matrix_load(char *filename);
matrix *matrix_load_part(char *filename, long long which, long long from_how_much);
void matrix_get_size(char *filename, long long *row_count, long long *column_count);
MPI_File matrix_get_file_started_from_part(char *filename, long long which, long long from_how_much, long long *needed_to_read_row_count, long long *column_count);
void matrix_save(matrix *m, char *filename);
void matrix_gen_and_save(long long row_count, long long column_count, char *filename);
void mpi_matrix_gen_and_save(long long row_count, long long column_count, char *filename);
void matrix_print(matrix *m);
void matrix_delete(matrix *m);

//multiplication operations
double dot(vector *v1, vector *v2);
vector *multiply(matrix *m, vector *v);
vector *mpi_multiply(char *matrixname, char *vectorname);
long long count_part(long long which, long long from_how_much, long long from_what);
void mpi_multiply_and_save_matrix(char *A_matrixname, char *B_matrixname, char *resultname);
MPI_File get_file_with_offset_for_write(char *filename, long long offset);

#endif

