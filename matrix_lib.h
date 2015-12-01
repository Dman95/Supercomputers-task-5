#ifndef MATRIX_LIB_H
#define MATRIX_LIB_H

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
void matrix_save(matrix *m, char *filename);
void matrix_gen_and_save(long long row_count, long long column_count, char *filename);
void matrix_print(matrix *m);
void matrix_delete(matrix *m);


//multiplication operations
double dot(vector *v1, vector *v2);
vector *multiply(matrix *m, vector *v);

#endif

