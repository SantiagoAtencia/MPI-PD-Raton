#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define SIZE 8
#define RATON 0
#define GATO 1

int movimientos[8][2] = {
    {-1, 0}, {-1, 1}, {0, 1}, {1, 1},
    {1, 0}, {1, -1}, {0, -1}, {-1, -1}
};

void mover(int *pos, int matriz[SIZE][SIZE]) {
    int nx, ny;
    while (1) {
        int dir = rand() % 8; // Dirección aleatoria
        nx = pos[0] + movimientos[dir][0];
        ny = pos[1] + movimientos[dir][1];

        if (nx >= 0 && ny >= 0 && nx < SIZE && ny < SIZE && matriz[nx][ny] == 1) {
            pos[0] = nx;
            pos[1] = ny;
            break;
        }
    }
}


int main(int argc,char *argv[]) {
    srand (time(NULL));
    int numtasks, rank;
    int sendbuf[SIZE * SIZE];
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    if (numtasks < 2) {
    if (rank == 0) {
        printf("Se necesitan al menos 2 procesos.\n");
    }
    MPI_Finalize();
    return -1;
}

    int filas_por_nodo = SIZE / numtasks;
    int filas_restantes = SIZE % numtasks;

    //Si el número de nodo es menor que el de filas restantes se le asigna una fila más a ese nodo
    int filas = filas_por_nodo + (rank < filas_restantes ? 1 : 0);

    int recvcount = filas*SIZE;

    int *recvbuf = (int *)malloc(filas * SIZE * sizeof(int));
    int *sendcounts = (int *)malloc(numtasks * sizeof(int));
    int *desplazamiento = (int *)malloc(numtasks * sizeof(int));
    int pos_raton[2] = {0, 0};
    int pos_gato[2] = {SIZE - 1, SIZE - 1};
    int victoria = 0; // 0: nadie ha ganado, 1: ratón gana, 2: gato gana
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

    int matriz[SIZE][SIZE];
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

                //printf("%d ", sendbuf[i * SIZE + j]);
                matriz[i][j]=sendbuf[i * SIZE + j];
                printf("%i ", matriz[i][j]);
            }
            printf("\n");
        }

        //Comprobar estado del laberinto y corregir
    }
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == GATO || rank == RATON) {
    MPI_Request request_send, request_recv;
    MPI_Status status;
    int received_flag;

    while (victoria == 0) {
        if (rank == RATON) {
            mover(pos_raton, matriz);
            printf("Ratón se mueve a (%d, %d)\n", pos_raton[0], pos_raton[1]);

            // Enviar la posición del ratón al gato (no bloqueante)
            MPI_Isend(pos_raton, 2, MPI_INT, GATO, 0, MPI_COMM_WORLD, &request_send);


            // Simular un tiempo arbitrario antes de continuar
            sleep(rand() % 3 + 1); // Tiempo aleatorio entre 1 y 3 segundos

            // Preparar para recibir la condición de victoria (no bloqueante)
            int victoria_local;
            MPI_Irecv(&victoria_local, 1, MPI_INT, GATO, 0, MPI_COMM_WORLD, &request_recv);

            // Esperar a que la recepción sea completada
            //MPI_Test(&request_recv, &received_flag, &status);
            while (!received_flag) {
                MPI_Test(&request_recv, &received_flag, &status);
            }

            if (victoria_local == 1) {
                printf("¡El ratón gana!\n");
            } else if (victoria_local == 2) {
                printf("¡El gato gana!\n");
            }

        } 
        if (rank == GATO) {
            mover(pos_gato, matriz);
            printf("Gato se mueve a (%d, %d)\n", pos_gato[0], pos_gato[1]);

            // Preparar para recibir la posición del ratón (no bloqueante)
            MPI_Irecv(pos_raton, 2, MPI_INT, RATON, 0, MPI_COMM_WORLD, &request_recv);

            // Esperar a que la recepción sea completada
           // MPI_Test(&request_recv, &received_flag, &status);
            while (!received_flag) {
                MPI_Test(&request_recv, &received_flag, &status);
            }

            // Verificar condición de victoria
            if (pos_gato[0] == pos_raton[0] && pos_gato[1] == pos_raton[1]) {
                victoria = 2;
            } else if (pos_raton[0] == 0 && pos_raton[1] == SIZE - 1) {
                victoria = 1;
            } else {
                victoria = 0;
            }

            // Enviar el resultado de la verificación al ratón (no bloqueante)
            MPI_Isend(&victoria, 1, MPI_INT, RATON, 0, MPI_COMM_WORLD, &request_send);

            // Simular un tiempo arbitrario antes de continuar
            sleep(rand() % 3 + 1); // Tiempo aleatorio entre 1 y 3 segundos

            if (victoria != 0) {
                if (victoria == 2) {
                    printf("¡El gato gana!\n");
                } else if (victoria == 1) {
                    printf("¡El ratón gana!\n");
                }
            }
        }

    }
}
    free(sendcounts);
    free(desplazamiento);
    free(recvbuf);

    MPI_Finalize();
}