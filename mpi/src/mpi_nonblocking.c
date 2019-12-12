/*
  Deberá implementar un algoritmo de multiplicación de matrices cuadradas usando cuatro procesos MPI y dividiendo los datos de cada una de las matrices en 4 bloques cuadrados, cada uno de los cuales será poseido por cada uno de los procesos.

  Por ejemplo, para matrices A, B y C de 2000x2000, donde C= A x B , cada uno de los procesos es dueño de un bloque de A de 500 x 500, de uno de B y de uno de C de igual tamaño. El proceso dueño de cada bloque de C es quien realiza el cómputo, y como C está distribuida entre los 4 procesos, todos computan en paralelo recibiendo los bloques desde los otros procesos.

  Haga un algoritmo que utilice primitivas bloqueantes, y otro no bloqueantes y verifique rendimiento.

  Plus: puede utilizar openmp en la parte del procesamiento, pero debe ser performante.

  mpirun -np 4 ./a.out
*/

#include "mpi.h"
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

/* #define MAX_DIM 2000 */
/* #define MAX_DIM 8 */
#define PROCS 4

char* asd;
unsigned int MAX_DIM;

void print_array(int dim, int c[dim][dim]);
void matmul(int partial_size, int aa[partial_size][partial_size], int bb[partial_size][partial_size], int cc[partial_size][partial_size]);

int main(int argc, char* argv[])
{
    MAX_DIM = strtol(getenv("MAX_DIM"), &asd, 10);
    int rank, numtasks; //tag = 1;
    double start, end;
    int a[MAX_DIM][MAX_DIM];
    int b[MAX_DIM][MAX_DIM];
    int c[MAX_DIM][MAX_DIM];

    for (int i = 0; i < MAX_DIM; i++) {
        for (int j = 0; j < MAX_DIM; j++) {
            /* a[i][j] = i*MAX_DIM + j; */
            /* b[i][j] = i*MAX_DIM + j; */
            a[i][j] = 1;
            b[i][j] = 1;
        }
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* if (rank == 0){ */
    /*     print_array(MAX_DIM, a); */
    /*     print_array(MAX_DIM, b); */
    /* } */

    //TODO: deberia andar esta declaracion de los arreglos???? Parece que a partir de C99 si
    int partial_size = MAX_DIM / numtasks;
    int partial_size2 = partial_size * partial_size;
    int aa[MAX_DIM][MAX_DIM], bb[MAX_DIM][MAX_DIM], cc[MAX_DIM][MAX_DIM];
    /* int dd[MAX_DIM][MAX_DIM]; // TODO: solo test, borrar, o no? */

    MPI_Request req[numtasks * numtasks * partial_size], req_a[numtasks],
            req_b[numtasks];
    MPI_Datatype subarray, col_subarray, row_subarray;
    int sizes[2] = { MAX_DIM, MAX_DIM }; /* size of global array */
    int subsizes[2] = { partial_size, partial_size }; /* size of sub-region */
    int starts[2] = { 0, 0 }; /* let's say we're looking at region "0",
                                 which begins at index [0,0] */
        //esos no son tus comentarios ivan, no seas ladron

    MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_INT, &subarray);
    MPI_Type_create_resized(subarray, 0, partial_size * sizeof(int), &row_subarray);
    MPI_Type_commit(&row_subarray);

    MPI_Type_create_resized(subarray, 0, MAX_DIM * partial_size * sizeof(int), &col_subarray);
    MPI_Type_commit(&col_subarray);

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    MPI_Iscatter(
        a,
        1,
        row_subarray,
        aa,
        partial_size2,
        MPI_INT,
        0,
        MPI_COMM_WORLD,
        &req_a[0]);
    MPI_Iscatter(
        b,
        1,
        col_subarray,
        bb,
        partial_size2,
        MPI_INT,
        0,
        MPI_COMM_WORLD,
        &req_b[0]);

    MPI_Iscatter(
        a + partial_size * 0,
        1,
        row_subarray,
        *aa + (0 + 1) * partial_size2,
        partial_size2,
        MPI_INT,
        0,
        MPI_COMM_WORLD,
        &req_a[0 + 1]);

    MPI_Wait(req_a, MPI_STATUS_IGNORE);

    MPI_Iscatter(
        *b + partial_size * 0,
        1,
        col_subarray,
        *bb + (0 + 1) * partial_size2,
        partial_size2,
        MPI_INT,
        0,
        MPI_COMM_WORLD,
        &req_b[0 + 1]);

    MPI_Wait(req_b + 0, MPI_STATUS_IGNORE);

    matmul(
        partial_size,
        (int(*)[partial_size])(*aa),
        (int(*)[partial_size])(*bb + 0 * partial_size2),
        (int(*)[partial_size])(*cc + 0 * partial_size2));

    for (int i = 0; i < partial_size; i++) {
        int * ptr_out = *cc + (0 * numtasks + 0) * partial_size2;
        MPI_Ireduce(
            ptr_out + i * partial_size,
            *(c + 0 * partial_size + i) + 0 * partial_size,
            partial_size,
            MPI_INT,
            MPI_SUM,
            0,
            MPI_COMM_WORLD,
            req + i + (0 * numtasks + 0) * partial_size);
    }

    for (int col = 1; col < numtasks - 1; col++) {

        MPI_Iscatter(
            *b + partial_size * col,
            1,
            col_subarray,
            *bb + (col + 1) * partial_size2,
            partial_size2,
            MPI_INT,
            0,
            MPI_COMM_WORLD,
            &req_b[col + 1]);

        MPI_Wait(req_b + col, MPI_STATUS_IGNORE);

        matmul(
            partial_size,
            (int(*)[partial_size])(*aa),
            (int(*)[partial_size])(*bb + col * partial_size2),
            (int(*)[partial_size])(*cc + col * partial_size2));

        MPI_Waitall(partial_size, req + (0 * numtasks + col - 1) * partial_size, MPI_STATUS_IGNORE);
        for (int i = 0; i < partial_size; i++) {
            int * ptr_out = *cc + (0 * numtasks + col) * partial_size2;
            MPI_Ireduce(
                ptr_out + i * partial_size,
                *(c + 0 * partial_size + i) + col * partial_size,
                partial_size,
                MPI_INT,
                MPI_SUM,
                0,
                MPI_COMM_WORLD,
                req + i + (0 * numtasks + col) * partial_size);
        }
    }
    MPI_Wait(req_b + numtasks - 1, MPI_STATUS_IGNORE);

    matmul(
        partial_size,
        (int(*)[partial_size])(*aa),
        (int(*)[partial_size])(*bb + (numtasks - 1) * partial_size2),
        (int(*)[partial_size])(*cc + (numtasks - 1) * partial_size2));

    MPI_Waitall(partial_size, req + (0 * numtasks + (numtasks - 1) - 1) * partial_size, MPI_STATUS_IGNORE);
    for (int i = 0; i < partial_size; i++) {
        int * ptr_out = *cc + (0 * numtasks + (numtasks - 1)) * partial_size2;
        MPI_Ireduce(
            ptr_out + i * partial_size,
            *(c + 0 * partial_size + i) + (numtasks - 1) * partial_size,
            partial_size,
            MPI_INT,
            MPI_SUM,
            0,
            MPI_COMM_WORLD,
            req + i + (0 * numtasks + (numtasks - 1)) * partial_size);
    }

    for (int row = 1; row < numtasks - 1; row++) {
        MPI_Iscatter(
            a + partial_size * row,
            1,
            row_subarray,
            *aa + (row + 1) * partial_size2,
            partial_size2,
            MPI_INT,
            0,
            MPI_COMM_WORLD,
            &req_a[row + 1]);

        MPI_Wait(req_a + row, MPI_STATUS_IGNORE);

        MPI_Waitall(partial_size, req + (row * numtasks - 1) * partial_size, MPI_STATUS_IGNORE);

        matmul(
            partial_size,
            (int(*)[partial_size])(*aa + row * partial_size2),
            (int(*)[partial_size])(*bb + 0 * partial_size2),
            (int(*)[partial_size])(*cc + (row * numtasks + 0) * partial_size2));

        for (int i = 0; i < partial_size; i++) {
            int* ptr_out = *cc + (row * numtasks + 0) * partial_size2;
            MPI_Ireduce(
                ptr_out + i * partial_size,
                *(c + row * partial_size + i) + 0 * partial_size,
                partial_size,
                MPI_INT,
                MPI_SUM,
                0,
                MPI_COMM_WORLD,
                req + i + (row * numtasks + 0) * partial_size);
        }

        for (int col = 1; col < numtasks; col++) {

            matmul(
                partial_size,
                (int(*)[partial_size])(*aa + row * partial_size2),
                (int(*)[partial_size])(*bb + col * partial_size2),
                (int(*)[partial_size])(*cc + (row * numtasks + col) * partial_size2));

            for (int i = 0; i < partial_size; i++) {
                int * ptr_out = *cc + (row * numtasks + col) * partial_size2;
                MPI_Ireduce(
                    ptr_out + i * partial_size,
                    *(c + row * partial_size + i) + col * partial_size,
                    partial_size,
                    MPI_INT,
                    MPI_SUM,
                    0,
                    MPI_COMM_WORLD,
                    req + i + (row * numtasks + col) * partial_size);
            }
        }
    }

    MPI_Wait(req_a + (numtasks - 1), MPI_STATUS_IGNORE);

    MPI_Waitall(partial_size, req + ((numtasks - 1) * numtasks - 1) * partial_size, MPI_STATUS_IGNORE);

    matmul(
        partial_size,
        (int(*)[partial_size])(*aa + (numtasks - 1) * partial_size2),
        (int(*)[partial_size])(*bb + 0 * partial_size2),
        (int(*)[partial_size])(*cc + ((numtasks - 1) * numtasks + 0) * partial_size2));

    for (int i = 0; i < partial_size; i++) {
        int* ptr_out = *cc + ((numtasks - 1) * numtasks + 0) * partial_size2;
        MPI_Ireduce(
            ptr_out + i * partial_size,
            *(c + (numtasks - 1) * partial_size + i) + 0 * partial_size,
            partial_size,
            MPI_INT,
            MPI_SUM,
            0,
            MPI_COMM_WORLD,
            req + i + ((numtasks - 1) * numtasks + 0) * partial_size);
    }

    for (int col = 1; col < numtasks; col++) {

        matmul(
            partial_size,
            (int(*)[partial_size])(*aa + (numtasks - 1) * partial_size2),
            (int(*)[partial_size])(*bb + col * partial_size2),
            (int(*)[partial_size])(*cc + ((numtasks - 1) * numtasks + col) * partial_size2));

        for (int i = 0; i < partial_size; i++) {
            int * ptr_out = *cc + ((numtasks - 1) * numtasks + col) * partial_size2;
            MPI_Ireduce(
                ptr_out + i * partial_size,
                *(c + (numtasks - 1) * partial_size + i) + col * partial_size,
                partial_size,
                MPI_INT,
                MPI_SUM,
                0,
                MPI_COMM_WORLD,
                req + i + ((numtasks - 1) * numtasks + col) * partial_size);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();

    MPI_Type_free(&col_subarray);
    MPI_Type_free(&row_subarray);

    if (rank == 0) {
        float time = end - start;
        /* printf("\n RESULTADO FINAL\n"); */
        /* print_array(MAX_DIM, c); */
        printf("Runtime = %f\n", time);
    }

    MPI_Finalize();
}

void print_array(int dim, int c[dim][dim])
{
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            printf(" %6d", c[i][j]);
        }
        printf("\n");
    }
}

void matmul(int partial_size, int aa[partial_size][partial_size], int bb[partial_size][partial_size], int cc[partial_size][partial_size])
{
    /* #pragma omp parallel for */
    int* ptr;
    register int tmp = 0;
    for (int j = 0; j < partial_size; j++) {
        for (int i = 0; i < partial_size; i++) {
            tmp = 0;
            ptr = aa[i];
            for (int k = 0; k < partial_size; k++) {
                tmp += *(ptr + k) * bb[k][j];
            }
            cc[i][j] = tmp;
        }
    }
}
