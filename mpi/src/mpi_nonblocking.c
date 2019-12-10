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
char* asd;
unsigned int MAX_DIM;
/* #define MAX_DIM 1500 */
/* #define MAX_DIM 16 */
#define PROCS 4

void print_array(int dim, int c[dim][dim]);

void matmul(int dim, int aa[dim][dim], int bb[dim][dim], int cc[dim][dim]);

void matmul_v2(int dim, int max_dim, int aa[dim][dim], int bb[dim][dim],
    int cc[max_dim][max_dim]);

void print_array_v2(int dim1, int dim2, int c[dim1][dim2]);

int main(int argc, char* argv[])
{
    MAX_DIM = strtol(getenv("MAX_DEPTH"), &asd, 10);
    int rank, numtasks; //tag = 1;
    double start, end;
    int a[MAX_DIM][MAX_DIM];
    int b[MAX_DIM][MAX_DIM];
    int c[MAX_DIM][MAX_DIM];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
        for (int i = 0; i < MAX_DIM; i++) {
            for (int j = 0; j < MAX_DIM; j++) {
                /* a[i][j] = i * MAX_DIM + j; */
                /* b[i][j] = i * MAX_DIM + j; */
                /* a[i][j] = j/2; */
                /* b[i][j] = i/2; */
                a[i][j] = 1;
                b[i][j] = 1;
            }
        }
    else
        for (int i = 0; i < MAX_DIM; i++) {
            for (int j = 0; j < MAX_DIM; j++) {
                a[i][j] = -1;
                b[i][j] = -1;
            }
        }

    /* if (rank == 0){ */
    /*     print_array(MAX_DIM, a); */
    /*     print_array(MAX_DIM, b); */
    /* } */

    //TODO: deberia andar esta declaracion de los arreglos???? Parece que a partir de C99 si
    int partial_size = MAX_DIM / numtasks;
    int partial_size2 = partial_size * partial_size;

    MPI_Request req[numtasks * numtasks * partial_size], req_a[numtasks],
        req_b[numtasks];
    MPI_Datatype subarray, col_subarray, row_subarray;
    int sizes[2] = { MAX_DIM, MAX_DIM }; /* size of global array */
    int subsizes[2] = { partial_size, partial_size }; /* size of sub-region */
    int starts[2] = { 0, 0 }; /* let's say we're looking at region "0",
                                 which begins at index [0,0] */
    //esos no son tus comentarios ivan, no seas ladron

    MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_INT,
        &subarray);
    MPI_Type_create_resized(subarray, 0, partial_size * sizeof(int),
        &row_subarray);
    MPI_Type_commit(&row_subarray);

    MPI_Type_create_resized(subarray, 0, MAX_DIM * partial_size * sizeof(int),
        &col_subarray);

    MPI_Type_commit(&col_subarray);

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    // ENVIO DE DATOS
    if (rank == 0) {
        for (int i = 0; i < numtasks; i++) {
            MPI_Iscatter(
                a + partial_size * i,
                1,
                row_subarray,
                MPI_IN_PLACE,
                partial_size2,
                MPI_INT,
                0,
                MPI_COMM_WORLD,
                req_a + i);
            MPI_Iscatter(
                *b + partial_size * i,
                1,
                col_subarray,
                MPI_IN_PLACE,
                partial_size * MAX_DIM,
                MPI_INT,
                0,
                MPI_COMM_WORLD,
                req_b + i);
        }
    } else {
        for (int i = 0; i < numtasks; i++) {
            MPI_Iscatter(
                0,
                1,
                row_subarray,
                *a + i * partial_size2,
                partial_size2,
                MPI_INT,
                0,
                MPI_COMM_WORLD,
                req_a + i);
            MPI_Iscatter(
                0,
                1,
                col_subarray,
                *b + i * partial_size2,
                partial_size * MAX_DIM,
                MPI_INT,
                0,
                MPI_COMM_WORLD,
                req_b + i);
        }
    }
    /* if (rank == 0) */
    /*   printf("FINALIZACION ENVIO DATOS \n"); */

    if (rank == 0) {
        MPI_Wait(req_a, MPI_STATUS_IGNORE);
        for (int row = 0; row < numtasks; row++) {
            for (int col = 0; col < numtasks; col++) {

                matmul_v2(
                    partial_size,
                    MAX_DIM,
                    a + row * partial_size,
                    (int(*)[MAX_DIM])(*b + col * partial_size),
                    (int(*)[MAX_DIM])(*(c + row * partial_size) + col * partial_size));
                for (int i = 0; i < partial_size; i++)
                    MPI_Ireduce(
                        MPI_IN_PLACE,
                        /* &c[row * partial_size + i][col * partial_size], */
                        *(c + row * partial_size + i) + col * partial_size,
                        partial_size,
                        MPI_INT,
                        MPI_SUM,
                        0,
                        MPI_COMM_WORLD,
                        req + i + row * partial_size + col);
            }
        }
    } else {
        for (int row = 0; row < numtasks; row++) {
            MPI_Wait(req_a + row, MPI_STATUS_IGNORE);
            for (int col = 0; col < numtasks; col++) {
                if (!row)
                    MPI_Wait(req_b + col, MPI_STATUS_IGNORE);

                matmul(
                    partial_size,
                    (int(*)[partial_size])(*a + row * partial_size2),
                    (int(*)[partial_size])(*b + col * partial_size2),
                    (int(*)[partial_size])(*(c + row) + col * partial_size2));

                for (int i = 0; i < partial_size; i++)
                    MPI_Ireduce(
                        *(c + row) + col * partial_size2 + i * partial_size,
                        0,
                        partial_size,
                        MPI_INT,
                        MPI_SUM,
                        0,
                        MPI_COMM_WORLD,
                        req + i + row * partial_size + col);
                /* &req[0]); */
            }
        }
    }
    /* for (int row = 0; row < numtasks; row++) { */
    /*     if (rank != 0) MPI_Wait(req_a + row, MPI_STATUS_IGNORE); */
    /*     for (int col = 0; col < numtasks; col++) { */
    /*         if (rank == 0) { */
    /*             matmul_v2( */
    /*                 partial_size, */
    /*                 MAX_DIM, */
    /*                 a + row * partial_size, */
    /*                 (int(*)[MAX_DIM])(*b + col * partial_size), */
    /*                 (int(*)[MAX_DIM])(*(c + row * partial_size) + col * partial_size)); */
    /*             for (int i = 0; i < partial_size; i++) */
    /*                 MPI_Ireduce( */
    /*                     MPI_IN_PLACE, */
    /*                     /\* &c[row * partial_size + i][col * partial_size], *\/ */
    /*                     *(c + row * partial_size + i) + col * partial_size, */
    /*                     partial_size, */
    /*                     MPI_INT, */
    /*                     MPI_SUM, */
    /*                     0, */
    /*                     MPI_COMM_WORLD, */
    /*                     req + i + row * partial_size + col); */
    /*                     /\* &req[0]); *\/ */
    /*         } else { */
    /*             if (!row) MPI_Wait(req_b + col, MPI_STATUS_IGNORE); */

    /*             matmul( */
    /*                 partial_size, */
    /*                 (int(*)[partial_size])(*a + row * partial_size2), */
    /*                 (int(*)[partial_size])(*b + col * partial_size2), */
    /*                 partial_size, */
    /*                 (int(*)[partial_size])(*(c + row) + col * partial_size2)); */

    /*             for (int i = 0; i < partial_size; i++) */
    /*                 MPI_Ireduce( */
    /*                     *(c + row) + col * partial_size2 + i * partial_size, */
    /*                     0, */
    /*                     partial_size, */
    /*                     MPI_INT, */
    /*                     MPI_SUM, */
    /*                     0, */
    /*                     MPI_COMM_WORLD, */
    /*                     req + i + row * partial_size + col); */
    /*                     /\* &req[0]); *\/ */
    /*         } */

    /*         /\* for (int i = 0; i < numtasks; i++) { *\/ */
    /*         /\*     MPI_Barrier(MPI_COMM_WORLD); *\/ */
    /*         /\*     if (rank == i) { *\/ */
    /*         /\*         if (rank == 0) { *\/ */
    /*         /\*             printf("COMIENZO REDUCCION ITERACION [%d][%d]\n", row, col); *\/ */
    /*         /\*             printf("\nRANK: %d\n", rank); *\/ */
    /*         /\*             printf("a: \n"); *\/ */
    /*         /\*             print_array_v2(partial_size, MAX_DIM, a + row * partial_size); *\/ */
    /*         /\*             printf("b: \n"); *\/ */
    /*         /\*             print_array_v2(partial_size, MAX_DIM, (int(*)[MAX_DIM]) (* b + col * partial_size)); *\/ */
    /*         /\*             printf("c: \n"); *\/ */
    /*         /\*             print_array_v2(partial_size, MAX_DIM, (int(*)[MAX_DIM])(*(c + row * partial_size) + col * partial_size)); *\/ */
    /*         /\*         } else { *\/ */
    /*         /\*             printf("\nRANK: %d\n", rank); *\/ */
    /*         /\*             printf("a: \n"); *\/ */
    /*         /\*             print_array(partial_size, (int(*)[partial_size])(*a + row * partial_size2)); *\/ */
    /*         /\*             printf("b: \n"); *\/ */
    /*         /\*             print_array(partial_size, (int(*)[partial_size])(*b + col * partial_size2)); *\/ */
    /*         /\*             printf("c: \n"); *\/ */
    /*         /\*             print_array(partial_size, (int(*)[partial_size])(*(c + row * partial_size) + col * partial_size)); *\/ */
    /*         /\*         } *\/ */
    /*         /\*     } *\/ */
    /*         /\* } *\/ */
    /*     } */
    /* } */

    /* MPI_Waitall(numtasks, req, MPI_STATUS_IGNORE); */
    /* if (rank == 0) */
    /*   printf("FIN MATMUL\n"); */

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

void print_array_v2(int dim1, int dim2, int c[dim1][dim2])
{
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim1; j++) {
            printf(" %6d", c[i][j]);
        }
        printf("\n");
    }
}

void matmul(int partial_size, int aa[partial_size][partial_size], int bb[partial_size][partial_size], int cc[partial_size][partial_size])
{
    //TODO: Optimizar esto
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
/* void matmul(int dim, int aa[dim][dim], int bb[dim][dim], int cc[dim][dim]) */
/* { */
/*     /\* #pragma omp parallel for *\/ */
/*     register int tmp = 0; */
/*     int* ptr; */
/*     for (int i = 0; i < dim; i++) { */
/*         for (int j = 0; j < dim; j++) { */
/*             tmp = 0; */
/*             ptr = aa[i]; */
/*             for (int k = 0; k < dim; k++) { */
/*                 tmp += *(ptr + k) * bb[k][j]; */
/*             } */
/*             cc[i][j] = tmp; */
/*         } */
/*     } */
/* } */

void matmul_v2(int dim, int max_dim, int aa[dim][max_dim], int bb[dim][max_dim], int cc[max_dim][max_dim])
{
    /* #pragma omp parallel for */
    register int tmp = 0;
    int* ptr;
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            tmp = 0;
            ptr = aa[i];
            for (int k = 0; k < dim; k++) {
                tmp += *(ptr + k) * bb[k][j];
            }
            cc[i][j] = tmp;
        }
    }
}
