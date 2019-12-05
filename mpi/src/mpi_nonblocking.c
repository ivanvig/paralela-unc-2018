/*
  Deberá implementar un algoritmo de multiplicación de matrices cuadradas usando cuatro procesos MPI y dividiendo los datos de cada una de las matrices en 4 bloques cuadrados, cada uno de los cuales será poseido por cada uno de los procesos.

  Por ejemplo, para matrices A, B y C de 2000x2000, donde C= A x B , cada uno de los procesos es dueño de un bloque de A de 500 x 500, de uno de B y de uno de C de igual tamaño. El proceso dueño de cada bloque de C es quien realiza el cómputo, y como C está distribuida entre los 4 procesos, todos computan en paralelo recibiendo los bloques desde los otros procesos.

  Haga un algoritmo que utilice primitivas bloqueantes, y otro no bloqueantes y verifique rendimiento.

  Plus: puede utilizar openmp en la parte del procesamiento, pero debe ser performante.

  mpirun -np 4 ./a.out
*/

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include<string.h>

#define MAX_DIM 2000
/* #define MAX_DIM 8 */
#define PROCS 4


void print_array(int dim, int c[dim][dim]);
void matmul(int dim, int aa[dim][dim], int bb[dim][dim], int max_dim, int cc[max_dim][max_dim]);
void print_array_v2(int dim1, int dim2, int c[dim1][dim2]);

int main (int argc, char *argv[])
{
    int rank, numtasks; //tag = 1;
    double start, end;
    int a[MAX_DIM][MAX_DIM];
    int b[MAX_DIM][MAX_DIM];
    int c[MAX_DIM][MAX_DIM];

    for (int i = 0; i< MAX_DIM; i++){
        for (int j = 0; j< MAX_DIM; j++){
            a[i][j] = i*MAX_DIM + j;
            b[i][j] = i*MAX_DIM + j;
            /* a[i][j] = 1; */
            /* b[i][j] = 1; */
        }
    }



    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    MPI_Request req[numtasks], req_a[numtasks], req_b[numtasks];
    /* MPI_Request req[4], req_a[4], req_b[4]; */
    /* MPI_Status stats[numtasks]; */
    /* if (rank == 0){ */
    /*     print_array(MAX_DIM, a); */
    /*     print_array(MAX_DIM, b); */
    /* } */

    //TODO: deberia andar esta declaracion de los arreglos???? Parece que a partir de C99 si
    int partial_size = MAX_DIM/numtasks;
    int aa_aux[MAX_DIM][partial_size], bb_aux[MAX_DIM][partial_size], cc_aux[MAX_DIM][MAX_DIM];
    /* int aa[partial_size][partial_size], bb[partial_size][partial_size], cc[partial_size][partial_size]; */
    /* int dd[partial_size][partial_size]; // TODO: solo test, borrar, o no? */

    MPI_Datatype subarray, col_subarray, row_subarray;
    int sizes[2]    = {MAX_DIM, MAX_DIM};  /* size of global array */
    int subsizes[2] = {partial_size, partial_size};  /* size of sub-region */
    int starts[2]   = {0,0};  /* let's say we're looking at region "0",
                                 which begins at index [0,0] */ //esos no son tus comentarios ivan, no seas ladron

    MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_INT, &subarray);
    MPI_Type_create_resized(subarray, 0, partial_size*sizeof(int), &row_subarray);
    MPI_Type_commit(&row_subarray);

    MPI_Type_create_resized(subarray, 0, MAX_DIM*partial_size*sizeof(int), &col_subarray);
    MPI_Type_commit(&col_subarray);

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    // ENVIO DE DATOS
    for (int i = 0; i < numtasks; i++) {
        //TODO: usar in place
        MPI_Iscatter(
            a + partial_size*i,
            1,
            row_subarray,
            *aa_aux + i*partial_size*partial_size,
            partial_size * partial_size,
            MPI_INT,
            0,
            MPI_COMM_WORLD,
            req_a + i
            );

        MPI_Iscatter(
            *b + partial_size*i,
            1,
            col_subarray,
            *bb_aux + i*partial_size*partial_size,
            partial_size * partial_size,
            MPI_INT,
            0,
            MPI_COMM_WORLD,
            req_b + i
            );
    }
    /* if (rank == 0) */
    /*   printf("FINALIZACION ENVIO DATOS \n"); */

    for (int row = 0; row < numtasks; row++) {
        MPI_Wait(req_a + row, MPI_STATUS_IGNORE);
        for (int col = 0; col < numtasks; col++) {
            //TODO: haciendo wait mas veces de las necesarias
            MPI_Wait(req_b + col, MPI_STATUS_IGNORE);

            matmul(
                partial_size,
                (int (*)[partial_size]) (*aa_aux + row*partial_size*partial_size),
                (int (*)[partial_size]) (*bb_aux + col*partial_size*partial_size),
                MAX_DIM,
                (int (*)[partial_size]) (*cc_aux + row*partial_size*MAX_DIM + col*partial_size)
                );

            /* if (rank == 0) */
            /*     printf("COMIENZO REDUCCION ITERACION [%d][%d]\n", row, col); */

            /* for (int i = 0; i < numtasks; i++) { */
            /*     MPI_Barrier(MPI_COMM_WORLD); */
            /*     if (rank == i) { */
            /*         printf("\nRANK: %d\n", rank); */
            /*         printf("aa: \n"); */
            /*         print_array(partial_size, aa_aux); */
            /*         printf("bb: \n"); */
            /*         print_array(partial_size, bb_aux); */
            /*         /\* printf("cc: \n"); *\/ */
            /*         /\* print_array_v2(MAX_DIM, partial_size, (int (*)[partial_size]) (*cc_aux + row*partial_size*MAX_DIM + col*partial_size)); *\/ */
            /*     } */
            /* } */
        }

        MPI_Ireduce(
            cc_aux[row * partial_size],
            c[row * partial_size],
            MAX_DIM*partial_size,
            MPI_INT,
            MPI_SUM,
            0,
            MPI_COMM_WORLD,
            &req[row]
            );
    }
    MPI_Waitall(numtasks, req, MPI_STATUS_IGNORE);
    /* if (rank == 0) */
    /*   printf("FIN MATMUL\n"); */

    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();

    MPI_Type_free(&col_subarray);
    MPI_Type_free(&row_subarray);

    if (rank == 0){
        float time = end-start;
        printf("\n RESULTADO FINAL\n");
        print_array(MAX_DIM, c);
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
        printf ("\n");
    }
}

void print_array_v2(int dim1, int dim2, int c[dim1][dim2])
{
    for (int i = 0; i < dim2; i++) {
        for (int j = 0; j < dim2; j++) {
            printf(" %6d", c[i][j]);
        }
        printf ("\n");
    }
}

void matmul(int dim, int aa[dim][dim], int bb[dim][dim], int max_dim, int cc[max_dim][max_dim])
{
#pragma omp parallel for
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            cc[i][j] = 0;
            for (int k = 0; k < dim; k++) {
                cc[i][j] += aa[i][k] * bb[k][j];
            }
        }
    }
}
