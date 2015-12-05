#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"
#include "sparse_lib.h"
#include <math.h>
#include <string.h>

//vector operations
sparse_vector *sparse_vector_new(long long size)
{
    sparse_vector *v = calloc(1, sizeof(sparse_vector));
    v->non_zero_count = 0;
    v->size = size;
    v->values = calloc(size, sizeof(sparse_value));
    return v;
}

double uniform_random()
{
    return ((double) rand()) / RAND_MAX;
}

long long geom_random(double non_zero_probability, long long max)
{
    double r = log(uniform_random()) / log(1 - non_zero_probability);
    return (r > max) ? max : ((long long) r);
}

sparse_vector *sparse_vector_gen(long long size, double non_zero_probability)
{
    sparse_vector *v = sparse_vector_new(size);
    for (long long i = geom_random(non_zero_probability, size); 
                   i < size; 
                   i += 1 + geom_random(non_zero_probability, size)) {
        double r =  ((double) rand()) / RAND_MAX;
        sparse_vector_push_back(v, i, r);
    }
    sparse_vector_shrink_to_fit(v);
    return v;
}

sparse_vector *sparse_vector_load(char *filename)
{
    MPI_File f;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
    
    //load vector
    sparse_vector *v = sparse_vector_load_fp(&f);
    
    MPI_File_close(&f);
    return v;
}

sparse_vector *sparse_vector_load_fp(MPI_File *f)
{
    MPI_Status s;

    //load size
    long long size;
    MPI_File_read(*f, &size, 1, MPI_LONG_LONG, &s);
    sparse_vector *v = sparse_vector_new(size);
    
    //load values
    MPI_File_read(*f, v->values, size * sizeof(sparse_value), MPI_CHAR, &s);
    v->non_zero_count = size;

    return v;
}

long long sparse_vector_serialized_size(sparse_vector *v)
{
    return sizeof(long long) + sizeof(sparse_value) * v->non_zero_count;
}

void sparse_vector_save(sparse_vector *v, char *filename)
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f); 
    
    //save vector
    sparse_vector_save_fp(v, &f);

    MPI_File_close(&f);
}

void sparse_vector_save_fp(sparse_vector *v, MPI_File *f)
{
    MPI_Status s;

    //save size
    MPI_File_write(*f, &v->non_zero_count, 1, MPI_LONG_LONG, &s);

    //save values
    MPI_File_write(*f, v->values, v->non_zero_count * sizeof(sparse_value), MPI_CHAR, &s);
}

void sparse_vector_push_back(sparse_vector *v, long long index, double value)
{
    v->values[v->non_zero_count].index = index;
    v->values[v->non_zero_count].value = value;
    v->non_zero_count++;
}

void sparse_vector_shrink_to_fit(sparse_vector *v)
{
    v->values = realloc(v->values, v->non_zero_count * sizeof(sparse_value));
    v->size = v->non_zero_count;
}

void sparse_vector_print(sparse_vector *v)
{
    printf("Sparse vector 1 x %lld (capacity %lld)\n", v->non_zero_count, v->size);
    printf("[ ");
    for (long long j = 0; j < v->non_zero_count; ++j) {
        printf("(%lld, %f) ", v->values[j].index, v->values[j].value);
        if (j > 10) {
            printf("... ");
            break;
        }
    }
    printf("]\n");
}

char *sparse_vector_serialize(sparse_vector *v)
{
    long long size = sparse_vector_serialized_size(v);
    char *buf = calloc(size, sizeof(char));
    *((long long *) buf) = v->non_zero_count;
    memcpy(buf + sizeof(long long), (char *) (v->values), size - sizeof(long long));
    return buf;
}

sparse_vector *sparse_vector_deserialize(char *serialized)
{
    long long non_zero_count = *((long long *) serialized);
    sparse_vector *v = sparse_vector_new(non_zero_count);
    memcpy((char *) (v->values), serialized + sizeof(long long), non_zero_count * sizeof(sparse_value));
    v->non_zero_count = v->size;
    return v;
}

void sparse_vector_delete(sparse_vector *v)
{
    free(v->values);
    free(v);
}


//matrix operations
sparse_matrix *sparse_matrix_new(long long row_count, long long column_count)
{
    sparse_matrix *m = sparse_matrix_new_without_vector_init(row_count, column_count);
    for (long long i = 0; i < row_count; ++i) {
        m->rows[i] = sparse_vector_new(column_count);
    }
    return m;
}

sparse_matrix *sparse_matrix_new_without_vector_init(long long row_count, long long column_count)
{
    sparse_matrix *m = calloc(1, sizeof(sparse_matrix));
    m->row_count = row_count;
    m->column_count = column_count;
    m->rows = calloc(row_count, sizeof(sparse_vector *));
    return m;
}

sparse_matrix *sparse_matrix_gen(long long row_count, long long column_count, double non_zero_probability)
{
    sparse_matrix *m = sparse_matrix_new_without_vector_init(row_count, column_count);
    for (long long i = 0; i < row_count; ++i) {
        m->rows[i] = sparse_vector_gen(column_count, non_zero_probability);
    }
    return m;
}

sparse_matrix *sparse_matrix_load(char *filename)
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);

    long long row_count, column_count;
    MPI_File_read(f, &row_count, 1, MPI_LONG_LONG, &s);
    MPI_File_read(f, &column_count, 1, MPI_LONG_LONG, &s);

    //omit offsets
    MPI_File_seek(f, row_count * sizeof(long long), MPI_SEEK_CUR);

    //load rows
    sparse_matrix *m = sparse_matrix_new_without_vector_init(row_count, column_count);
    for (long long i = 0; i < row_count; ++i) {
        sparse_vector *row = sparse_vector_load_fp(&f);
        m->rows[i] = row;
    }

    MPI_File_close(&f);
    return m;
}

sparse_matrix *sparse_matrix_load_part(char *filename, long long which, long long from_how_much) 
//e.g. which = 0, from_how_much = 4 should load 25% of rows
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);

    //read row and column counts
    long long row_count, column_count;
    MPI_File_read(f, &row_count, 1, MPI_LONG_LONG, &s);
    MPI_File_read(f, &column_count, 1, MPI_LONG_LONG, &s);

    //seek to our part of data
    long long one_part_size_in_rows = row_count / from_how_much;
    long long start_row = one_part_size_in_rows * which;
    //read start row offset
    MPI_File_seek(f, start_row * sizeof(long long), MPI_SEEK_CUR);
    long long start_row_offset = 0;
    MPI_File_read(f, &start_row_offset, 1, MPI_LONG_LONG, &s);
    MPI_File_seek(f, start_row_offset, MPI_SEEK_SET); 

    //count amount of data to read
    long long needed_to_read_row_count = count_part(which, from_how_much, row_count);

    //read data
    sparse_matrix *m = sparse_matrix_new_without_vector_init(needed_to_read_row_count, column_count);
    for (long long i = 0; i < needed_to_read_row_count; ++i) {
        sparse_vector *row = sparse_vector_load_fp(&f);
        m->rows[i] = row;
    }

    MPI_File_close(&f);
    return m;
}

void sparse_matrix_save(sparse_matrix *m, char *filename)
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f);

    MPI_File_write(f, &m->row_count, 1, MPI_LONG_LONG, &s);
    MPI_File_write(f, &m->column_count, 1, MPI_LONG_LONG, &s);

    //place for offsets
    long long current_offset = sizeof(long long) + sizeof(long long) + m->row_count * sizeof(long long);
    MPI_File_seek(f, current_offset, MPI_SEEK_SET);
    long long *offsets = calloc(m->row_count, sizeof(long long));

    //save rows
    for (long long i = 0; i < m->row_count; ++i) {
        offsets[i] = current_offset;
        sparse_vector_save_fp(m->rows[i], &f);
        current_offset += sparse_vector_serialized_size(m->rows[i]);
    }

    //save offsets
    MPI_File_seek(f, sizeof(long long) + sizeof(long long), MPI_SEEK_SET);
    MPI_File_write(f, offsets, m->row_count, MPI_LONG_LONG, &s);
    free(offsets);

    MPI_File_close(&f);
}

void sparse_matrix_gen_and_save(long long row_count, long long column_count, char *filename, double non_zero_probability)
{
    MPI_File f;
    MPI_Status s;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f);
    
    MPI_File_write(f, &row_count, 1, MPI_LONG_LONG, &s);
    MPI_File_write(f, &column_count, 1, MPI_LONG_LONG, &s);

    //place for offsets
    long long current_offset = sizeof(long long) + sizeof(long long) + row_count * sizeof(long long);
    MPI_File_seek(f, current_offset, MPI_SEEK_SET);
    long long *offsets = calloc(row_count, sizeof(long long));

    //save rows
    for (long long i = 0; i < row_count; ++i) {
        offsets[i] = current_offset;
        sparse_vector *v = sparse_vector_gen(column_count, non_zero_probability);
        sparse_vector_save_fp(v, &f);
        current_offset += sparse_vector_serialized_size(v);
        sparse_vector_delete(v);
    }

    //save offsets
    MPI_File_seek(f, sizeof(long long) + sizeof(long long), MPI_SEEK_SET);
    MPI_File_write(f, offsets, row_count, MPI_LONG_LONG, &s);
    free(offsets);

    MPI_File_close(&f);
}

void mpi_sparse_matrix_gen_and_save(long long row_count, long long column_count, char *filename, double non_zero_probability)
{
    int myrank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (!myrank) { //root process which get results
        MPI_File f;
        MPI_Status s;
        MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f);
    
        MPI_File_write(f, &row_count, 1, MPI_LONG_LONG, &s);
        MPI_File_write(f, &column_count, 1, MPI_LONG_LONG, &s);

        //place for offsets
        long long current_offset = sizeof(long long) + sizeof(long long) + row_count * sizeof(long long);
        MPI_File_seek(f, current_offset, MPI_SEEK_SET);
        long long *offsets = calloc(row_count, sizeof(long long));

        //get and save rows
        long long buf_length = sizeof(long long) + column_count * sizeof(sparse_value);
        char *buf = calloc(buf_length, 1);
        for (long long i = 0; i < row_count; ++i) {
            MPI_Recv(buf, buf_length, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
            sparse_vector *v = sparse_vector_deserialize(buf);
            
            offsets[i] = current_offset;
            sparse_vector_save_fp(v, &f);
            current_offset += sparse_vector_serialized_size(v);

            sparse_vector_delete(v);
        }

        //save offsets
        MPI_File_seek(f, sizeof(long long) + sizeof(long long), MPI_SEEK_SET);
        MPI_File_write(f, offsets, row_count, MPI_LONG_LONG, &s);
        free(offsets);

        MPI_File_close(&f);
    } else { //other processes which generate results
        --size;
        --myrank;
        long long needed_to_generate_row_count = count_part(myrank, size, row_count);
        for (long long i = 0; i < needed_to_generate_row_count; ++i) {
            sparse_vector *v = sparse_vector_gen(column_count, non_zero_probability);
            char *serialized = sparse_vector_serialize(v);
            long long serialized_size = sparse_vector_serialized_size(v);
            MPI_Send(serialized, serialized_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            free(serialized);
            sparse_vector_delete(v);
        }
    }
}

void sparse_matrix_print(sparse_matrix *m)
{
    printf("Sparse matrix %lld x %lld\n", m->row_count, m->column_count);
    for (long long i = 0; i < m->row_count; ++i) {
        if (i > 10) {
            printf("[ ... ]\n");
            break;
        }
        printf("[");
        for (long long j = 0; j < m->rows[i]->size; ++j) {
            printf("(%lld, %f) ", m->rows[i]->values[j].index, m->rows[i]->values[j].value);
            if (j > 10) {
                printf("... ");
                break;
            }
        }
        printf("]\n");
    }
}

void sparse_matrix_delete(sparse_matrix *m)
{
    for (long long i = 0; i < m->row_count; ++i) {
        sparse_vector_delete(m->rows[i]);
    }
    free(m->rows);
    free(m);
}


//multiplication functions
double sparse_dot(sparse_vector *v1, vector *v2)
{
    double result = 0;
    for (long long i = 0; i < v1->non_zero_count; ++i) {
        result += v1->values[i].value * v2->values[v1->values[i].index]; 
    }
    return result;
}

vector *sparse_multiply(sparse_matrix *m, vector *v)
{
    vector *result = vector_new(m->row_count);
    for (long long i = 0; i < m->row_count; ++i) {
        result->values[i] = sparse_dot(m->rows[i], v);
    }
    return result;
}

vector *mpi_sparse_multiply(char *matrixname, char *vectorname)
{   
    int myrank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    //count result
    sparse_matrix *m = sparse_matrix_load_part(matrixname, myrank, size);
    vector *v = vector_load(vectorname);
    vector *result = sparse_multiply(m, v);
    
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
    sparse_matrix_delete(m);
    vector_delete(v);
    vector_delete(result);

    return final_result;
}

