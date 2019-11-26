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

/* #define MAX_DIM 2000 */
#define MAX_DIM 8
#define PROCS 4


void print_array(int dim, int c[dim][dim]);

int main (int argc, char *argv[])
{
    int rank, numtasks, sum = 0; //tag = 1;
    int a[MAX_DIM][MAX_DIM];
    int b[MAX_DIM][MAX_DIM];
    int c[MAX_DIM][MAX_DIM];

    for (int i = 0; i< MAX_DIM; i++){
        for (int j = 0; j< MAX_DIM; j++){
            a[i][j] = i*MAX_DIM + j;
            b[i][j] = i*MAX_DIM + j;
        }
    }



    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if (rank == 0){
        print_array(MAX_DIM, a);
        print_array(MAX_DIM, b);
    }

    int aa[MAX_DIM/numtasks][MAX_DIM/numtasks], bb[MAX_DIM/numtasks], cc[MAX_DIM/numtasks];
    for (int i = 0; i < MAX_DIM/numtasks; i++){
        MPI_Scatter(a + i, MAX_DIM/numtasks, MPI_INT, aa + i, MAX_DIM/numtasks, MPI_INT, 0,MPI_COMM_WORLD);
    }

    if (rank == 1){
        print_array(MAX_DIM/numtasks, aa);
        /* print_array(b); */
    }

    /* //No es necesario, todos tienen una copia */
    /* /\* MPI_Bcast(b, MAX_DIM*MAX_DIM, MPI_INT, 0, MPI_COMM_WORLD); *\/ */

    /* MPI_Barrier(MPI_COMM_WORLD); */

    /* for (int i = 0; i < MAX_DIM; i++) */
    /*     { */
    /*         for (int j = 0; j < MAX_DIM; j++) */
    /*             { */
    /*                 sum = sum + aa[j] * b[j][i]; */
    /*             } */
    /*         cc[i] = sum; */
    /*         sum = 0; */
    /*         } */

    /* MPI_Gather(cc, MAX_DIM*MAX_DIM/numtasks, MPI_INT, c, MAX_DIM*MAX_DIM/numtasks, MPI_INT, 0, MPI_COMM_WORLD); */
    /* MPI_Barrier(MPI_COMM_WORLD); */

    /* if (rank == 0) */
    /*     print_array(c); */

    MPI_Finalize();
}


/* void print_array(int c[MAX_DIM][MAX_DIM]) */
/* { */
/*     for (int i = 0; i < MAX_DIM; i++) { */
/*         for (int j = 0; j < MAX_DIM; j++) { */
/*             printf(" %d", c[i][j]); */
/*         } */
/*         printf ("\n"); */
/*     } */
/* } */
void print_array(int dim, int c[dim][dim])
{
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            printf(" %d", c[i][j]);
        }
        printf ("\n");
    }
}
