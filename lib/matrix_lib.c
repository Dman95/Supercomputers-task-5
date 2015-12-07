#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"
#include "matrix_lib.h"

//vector operations
vector *vector_new(long long size)
{
    vector *v = calloc(1, sizeof(vector));
    v->size = size;
    if (size) {
        v->values = calloc(size, sizeof(double));
    }
    return v;
}

vector *vector_gen(long long size)
{
    vector *v = vector_new(size);
    for (long long i = 0; i < size; ++i) {
        v->values[i] = ((double) rand()) / RAND_MAX;
    }
    return v;
}

vector *vector_load(char *filename)
{
    MPI_File f;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
    
    //load vector
    vector *v = vector_load_fp(&f);
    
    MPI_File_close(&f);
    return v;
}

vector *vector_load_fp(MPI_File *f)
{
    MPI_Status s;

    //load size
    long long size;
    MPI_File_read(*f, &size, 1, MPI_LONG_LONG, &s);
    vector *v = vector_new(size);
    
    //load values
    MPI_File_read(*f, v->values, size, MPI_DOUBLE, &s);

    return v;
}

void vector_save(vector *v, char *filename)
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f); 
    
    //save vector
    vector_save_fp(v, &f);

    MPI_File_close(&f);
}

void vector_save_fp(vector *v, MPI_File *f)
{
    MPI_Status s;

    //save size
    MPI_File_write(*f, &v->size, 1, MPI_LONG_LONG, &s);

    //save values
    MPI_File_write(*f, v->values, v->size, MPI_DOUBLE, &s);
}

void vector_print(vector *v)
{
    printf("Vector 1 x %lld\n", v->size);
    printf("[ ");
    for (long long j = 0; j < v->size; ++j) {
        printf("%f ", v->values[j]);
        if (j > 10) {
            printf("... ");
            break;
        }
    }
    printf("]\n");
}

void vector_delete(vector *v)
{
    free(v->values);
    free(v);
}


//matrix operations
matrix *matrix_new(long long row_count, long long column_count)
{
    matrix *m = matrix_new_without_vector_init(row_count, column_count);
    for (long long i = 0; i < row_count; ++i) {
        m->rows[i] = vector_new(column_count);
    }
    return m;
}

matrix *matrix_new_without_vector_init(long long row_count, long long column_count)
{
    matrix *m = calloc(1, sizeof(matrix));
    m->row_count = row_count;
    m->column_count = column_count;
    if (row_count) {
        m->rows = calloc(row_count, sizeof(vector *));
    }
    return m;
}

matrix *matrix_gen(long long row_count, long long column_count)
{
    matrix *m = matrix_new_without_vector_init(row_count, column_count);
    for (long long i = 0; i < row_count; ++i) {
        m->rows[i] = vector_gen(column_count);
    }
    return m;
}

matrix *matrix_load(char *filename)
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);

    long long row_count, column_count;
    MPI_File_read(f, &row_count, 1, MPI_LONG_LONG, &s);
    MPI_File_read(f, &column_count, 1, MPI_LONG_LONG, &s);

    //load rows
    matrix *m = matrix_new_without_vector_init(row_count, column_count);
    for (long long i = 0; i < row_count; ++i) {
        vector *row = vector_load_fp(&f);
        m->rows[i] = row;
    }

    MPI_File_close(&f);
    return m;
}

void matrix_get_size(char *filename, long long *row_count, long long *column_count)
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);

    MPI_File_read(f, row_count, 1, MPI_LONG_LONG, &s);
    MPI_File_read(f, column_count, 1, MPI_LONG_LONG, &s);

    MPI_File_close(&f);
}

MPI_File matrix_get_file_started_from_part(char *filename, long long which, long long from_how_much, long long *needed_to_read_row_count, 
        long long *column_count)
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);

    //read row and column counts
    long long row_count;
    MPI_File_read(f, &row_count, 1, MPI_LONG_LONG, &s);
    MPI_File_read(f, column_count, 1, MPI_LONG_LONG, &s);

    //seek to our part of data
    *needed_to_read_row_count = count_part(which, from_how_much, row_count);
    long long one_part_size_in_rows = (row_count + from_how_much - 1) / from_how_much;
    long long row_size_in_bytes = sizeof(long long) + sizeof(double) * (*column_count);
    MPI_File_seek(f, row_size_in_bytes * one_part_size_in_rows * which, MPI_SEEK_CUR); 

    return f;
}

matrix *matrix_load_part(char *filename, long long which, long long from_how_much) 
//e.g. which = 0, from_how_much = 4 should load 25% of rows
{
    long long needed_to_read_row_count, column_count;
    MPI_File f = matrix_get_file_started_from_part(filename, which, from_how_much, &needed_to_read_row_count, &column_count);    

    //read data
    matrix *m = matrix_new_without_vector_init(needed_to_read_row_count, column_count);
    for (long long i = 0; i < needed_to_read_row_count; ++i) {
        vector *row = vector_load_fp(&f);
        m->rows[i] = row;
    }

    MPI_File_close(&f);
    return m;
}

void matrix_save(matrix *m, char *filename)
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f);

    MPI_File_write(f, &m->row_count, 1, MPI_LONG_LONG, &s);
    MPI_File_write(f, &m->column_count, 1, MPI_LONG_LONG, &s);

    //save rows
    for (long long i = 0; i < m->row_count; ++i) {
        vector_save_fp(m->rows[i], &f);
    }
    
    MPI_File_close(&f);
}

void mpi_matrix_gen_and_save(long long row_count, long long column_count, char *filename)
{
    int myrank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
   
    long long needed_to_generate_row_count = count_part(myrank, size, row_count); 

    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f);
    
    if (!myrank) {
        MPI_File_write(f, &row_count, 1, MPI_LONG_LONG, &s);
        MPI_File_write(f, &column_count, 1, MPI_LONG_LONG, &s);
    }

    long long row_size_in_bytes = sizeof(long long) + column_count * sizeof(double);
    long long my_row_offset = (row_count / size) * myrank;
    MPI_File_seek(f, 2 * sizeof(long long) + row_size_in_bytes * my_row_offset, MPI_SEEK_SET);

    //save rows
    for (long long i = 0; i < needed_to_generate_row_count; ++i) {
        vector *v = vector_gen(column_count);
        vector_save_fp(v, &f);
        vector_delete(v);
    }
    
    MPI_File_close(&f);
}

void matrix_gen_and_save(long long row_count, long long column_count, char *filename)
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f);
    
    MPI_File_write(f, &row_count, 1, MPI_LONG_LONG, &s);
    MPI_File_write(f, &column_count, 1, MPI_LONG_LONG, &s);

    //save rows
    for (long long i = 0; i < row_count; ++i) {
        vector *v = vector_gen(column_count);
        vector_save_fp(v, &f);
        vector_delete(v);
    }
    
    MPI_File_close(&f);
}

void matrix_print(matrix *m)
{
    printf("Matrix %lld x %lld\n", m->row_count, m->column_count);
    for (long long i = 0; i < m->row_count; ++i) {
        if (i > 10) {
            printf("[ ... ]\n");
            break;
        }
        printf("[");
        for (long long j = 0; j < m->column_count; ++j) {
            printf("%f ", m->rows[i]->values[j]);
            if (j > 10) {
                printf("... ");
                break;
            }
        }
        printf("]\n");
    }
}

void matrix_delete(matrix *m)
{
    for (long long i = 0; i < m->row_count; ++i) {
        vector_delete(m->rows[i]);
    }
    free(m->rows);
    free(m);
}

//multiplication functions
double dot(vector *v1, vector *v2)
{
    double result = 0;
    for (long long i = 0; i < v1->size; ++i) {
        result += v1->values[i] * v2->values[i];
    }
    return result;
}

vector *multiply(matrix *m, vector *v)
{
    vector *result = vector_new(m->row_count);
    for (long long i = 0; i < m->row_count; ++i) {
        result->values[i] = dot(m->rows[i], v);
    }
    return result;
}

//return non null only in root = 0 process
vector *mpi_multiply(char *matrixname, char *vectorname)
{
    int myrank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    //count result
    vector *v = vector_load(vectorname);
    
    long long needed_to_read_row_count, column_count;
    MPI_File matrix_file = matrix_get_file_started_from_part(matrixname, myrank, size, &needed_to_read_row_count, &column_count); 
    
    vector *result = vector_new(needed_to_read_row_count);
    for (long long i = 0; i < needed_to_read_row_count; ++i) {
        vector *row = vector_load_fp(&matrix_file);
        result->values[i] = dot(row, v);
        vector_delete(row);
    }

    MPI_File_close(&matrix_file);

    //gather result
    vector *final_result = NULL;
    int *recvcounts = NULL;
    int *displs = NULL;
    if (!myrank) {
        final_result = vector_new(v->size);
        recvcounts = calloc(size, sizeof(int));
        displs = calloc(size, sizeof(int));
        int count = v->size / size;
        int displ = 0;
        for (int i = 0; i < size - 1; ++i) {
            recvcounts[i] = count;
            displs[i] = displ;
            displ += count;
        }
        recvcounts[size - 1] = v->size - displ;
        displs[size - 1] = displ;
    }
    MPI_Gatherv(result->values, result->size, MPI_DOUBLE, (final_result != 0) ? final_result->values : 0, recvcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    //clean
    if (!myrank) {
        free(recvcounts);
        free(displs);
    }
    vector_delete(v);
    vector_delete(result);

    return final_result;
}

long long count_part(long long which, long long from_how_much, long long from_what)
{    
    long long one_part_size = (from_what + from_how_much - 1) / from_how_much;
    if (one_part_size * which + one_part_size <= from_what) {
        return one_part_size;
    }
    if (one_part_size * which >= from_what) {
        return 0;
    }
    long long remainder = from_what % one_part_size;
    return remainder;
}

void mpi_multiply_and_save_matrix(char *A_matrixname, char *B_matrixname, char *resultname)
{
    int myrank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //load A and B matrix parts
    matrix *A_part = matrix_load_part(A_matrixname, myrank, size);
    matrix *B_part = matrix_load_part(B_matrixname, myrank, size);
    long long A_row_count, A_column_count, B_row_count, B_column_count;
    matrix_get_size(A_matrixname, &A_row_count, &A_column_count);
    matrix_get_size(B_matrixname, &B_row_count, &B_column_count);
    if (A_column_count != B_row_count) {
        printf("Inconsistent column and row counts\n");
        exit(-1);
    }

    //go through all B columns and gather them
    //init displs and recvcounts
    int *displs = calloc(size, sizeof(int));
    int *recvcounts = calloc(size, sizeof(int));
    int offset = 0;
    for (int i = 0; i < size; ++i) {
        displs[i] = offset;
        recvcounts[i] = count_part(i, size, B_row_count);
        offset += recvcounts[i];
    }
    //count result matrix part
    matrix *result_part = matrix_new(A_part->row_count, B_column_count);
    vector *B_column_part = vector_new(B_part->row_count);
    vector *B_column = vector_new(B_row_count);
    for (long long i = 0; i < B_column_count; ++i) {
        //get column part
        for (long long j = 0; j < B_part->row_count; ++j) {
            B_column_part->values[j] = B_part->rows[j]->values[i];
        }
        //gather full column
        MPI_Allgatherv(B_column_part->values, B_column_part->size, MPI_DOUBLE, B_column->values, recvcounts, displs, 
                                                                   MPI_DOUBLE, MPI_COMM_WORLD);
        //count multiplication
        vector *result_part_column = multiply(A_part, B_column);
        for (long long j = 0; j < result_part_column->size; ++j) {
            result_part->rows[j]->values[i] = result_part_column->values[j];
        }
        vector_delete(result_part_column);
    }
    vector_delete(B_column);
    vector_delete(B_column_part);
    free(displs);
    free(recvcounts);

    //now we have part of rows of matrix B in result_part matrix, we can write it
    if (!myrank) {
        //write size
        MPI_File f;
        MPI_Status s;
        MPI_File_open(MPI_COMM_SELF, resultname, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f);
        MPI_File_write(f, &A_row_count, 1, MPI_LONG_LONG, &s);
        MPI_File_write(f, &B_column_count, 1, MPI_LONG_LONG, &s);
        MPI_File_close(&f);
    }
    MPI_File resultfile = get_file_with_offset_for_write(resultname, 2 * sizeof(long long) + 
            count_part(0, size, A_row_count) * myrank * (sizeof(long long) + B_column_count * sizeof(double)));
    for (long long i = 0; i < result_part->row_count; ++i) {
        vector_save_fp(result_part->rows[i], &resultfile);
    }
    MPI_File_close(&resultfile);
}

MPI_File get_file_with_offset_for_write(char *filename, long long offset)
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f);

    //seek to our part of data
    MPI_File_seek(f, offset, MPI_SEEK_SET); 

    return f;
}
