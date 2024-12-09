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

                // Corrección de la matriz generada

        // 1. Asegurar que las esquinas sean 1
        sendbuf[0] = 1;                                  // Esquina superior izquierda
        sendbuf[SIZE - 1] = 1;                           // Esquina superior derecha
        sendbuf[(SIZE - 1) * SIZE] = 1;                  // Esquina inferior izquierda
        sendbuf[(SIZE - 1) * SIZE + SIZE - 1] = 1;       // Esquina inferior derecha

        // 2. Corregir filas para evitar ceros consecutivos
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE - 1; ++j) {
                if (sendbuf[i * SIZE + j] == 0 && sendbuf[i * SIZE + j + 1] == 0) {
                    sendbuf[i * SIZE + j + 1] = 1; // Corregir para romper la secuencia de ceros
                }
            }
        }

        // 3. Asegurar que cada columna tenga al menos un 1
        for (int j = 0; j < SIZE; ++j) {
            int tiene_uno = 0;
            for (int i = 0; i < SIZE; ++i) {
                if (sendbuf[i * SIZE + j] == 1) {
                    tiene_uno = 1;
                    break;
                }
            }
            if (!tiene_uno) {
                // Si no hay un 1 en la columna, establecer 1 en la primera fila de esa columna
                sendbuf[j] = 1;
            }
        }


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