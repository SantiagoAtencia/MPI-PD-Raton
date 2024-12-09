#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SIZE 8


int main(int argc,char *argv[]) {
    srand (time(NULL));
    int numtasks, rank;
    int sendbuf[SIZE * SIZE];

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    
    int filas_por_nodo = SIZE / numtasks;
    int filas_restantes = SIZE % numtasks;

    //Si el número de nodo es menor que el de filas restantes se le asigna una fila más a ese nodo
    int filas = filas_por_nodo + (rank < filas_restantes ? 1 : 0);

    int recvcount = filas*SIZE;

    int *recvbuf = (int *)malloc(filas * SIZE * sizeof(int));
    int *sendcounts = (int *)malloc(numtasks * sizeof(int));
    int *desplazamiento = (int *)malloc(numtasks * sizeof(int));

    if(rank ==  0){
        for(int i = 0; i < numtasks; ++i){
            sendcounts[i] = (filas_por_nodo + (i < filas_restantes ? 1 : 0)) * SIZE;
            desplazamiento[i] = i * filas_por_nodo * SIZE + (i < filas_restantes ? i * SIZE : filas_restantes * SIZE);

            //Comprobación número de elementos para cada nodo y el desplazamiento que tendrá en el scatter.
            //printf("Sendcount: %d, Desplazamiento: %d\n", sendcounts[i], desplazamiento[i]);
        }
    }

    MPI_Scatterv(sendbuf,sendcounts,desplazamiento,MPI_INT,recvbuf,recvcount,MPI_INT,0,MPI_COMM_WORLD);

    for(int i = 0; i < SIZE * filas; ++i){
        recvbuf[i] = rand() % 2;
        srand (time(NULL)+rank+i);
    }

    // Comprobación filas que genera cada nodo
    /*
    printf("Rank = %d ---> ", rank);
    for(int i = 0; i < filas*SIZE; ++i){
        
        printf("%d ", recvbuf[i]);
        
    }
    printf("\n");
    //*/

    MPI_Gatherv(recvbuf,recvcount,MPI_INT,sendbuf,sendcounts,desplazamiento,MPI_INT,0,MPI_COMM_WORLD);

    if(rank == 0){
        for(int i = 0; i < SIZE; ++i){
            for(int j = 0; j < SIZE; ++j){
                printf("%d ", sendbuf[i * SIZE + j]);
            }
            printf("\n");
        }

        //Comprobar estado del laberinto y corregir
    }

    free(sendcounts);
    free(desplazamiento);
    free(recvbuf);

    MPI_Finalize();
}