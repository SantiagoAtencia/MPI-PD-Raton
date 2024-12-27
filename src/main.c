#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "maze.h"

#define MIN_DIM 4
#define MAX_DIM 100


//🐁🐁🐁🐁🐁🐭🐭🐭🐈🐈🐱🐱🕱🕱☠☠
//╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬
//╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬🐈╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬
//xxxxxxxxxxxxxxxx🐈xxxx╬╬    ╬╬╬╬╬╬╬  ╬╬╬╬╬╬
// ██████
// █    █
// ████  ▓▓▓
// █   ▓█ ▓▓▓▓🐁 █🐁
// ███▓▓▓▓ ▓ ▓ █ █ █ ▓ ▓▓▓▓▓



void print_usage() {
    printf("Usage: mpirun -np <num_processes> ./maze <width> <height>\n");
    printf("Both dimensions must be between %d and %d.\n", MIN_DIM, MAX_DIM);
    printf("Height must be a multiple of the number of processes.\n");
}

/**
 * Reads and checks the input arguments.
 */
void read_and_check_input(int argc, char** argv, int rank, int num_procs, int* width, int* height) {
    int valid_input = 1;

    if (rank == 0) {
        if (argc != 3) {
            print_usage();
            valid_input = 0;
        } else {
            char *endptr1, *endptr2;
            *width = strtol(argv[1], &endptr1, 10);
            *height = strtol(argv[2], &endptr2, 10);

            if (*endptr1 != '\0' || *endptr2 != '\0' || 
                *width < MIN_DIM || *width > MAX_DIM || 
                *height < MIN_DIM || *height > MAX_DIM || 
                *height % num_procs != 0) {
                print_usage();
                valid_input = 0;
            }
        }
        printf("Building a maze of dimensions %d x %d with %d processes.\n", *height, *width, num_procs);
    }

    // Broadcast the validity of the input to all processes:
    MPI_Bcast(&valid_input, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (!valid_input) {
        MPI_Finalize();
        exit(1);
    }
    // Broadcast the dimensions to all processes:
    MPI_Bcast(width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(height, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

/**
 * Generates a sub-maze for each process.
 * 
 * Each process generates a sub-maze of dimensions width x height/num_procs.
 */
Maze generate_sub_maze(int rank, int num_procs, int width, int height) {
    //allocate memory for the sub-maze
    int sub_height = height / num_procs;
    Maze sub_maze = create_maze(width, sub_height);
    // Fill the sub-maze with random walls and paths
    fill_maze_random(sub_maze,rank);
    return sub_maze;
}

/**
 * Merges the sub-mazes into the final maze.
 * 
 */
Maze merge_sub_mazes(Maze sub_maze, int rank, int num_procs) {
    Maze final_maze;
    if (rank == 0) {
        // Allocate memory for the final maze:
        final_maze = create_maze(sub_maze.width, sub_maze.height * num_procs);       
    }
    int sub_maze_size = sub_maze.width * sub_maze.height;
    // gather all sub-mazes into the final maze:
    MPI_Gather(sub_maze.cells, sub_maze_size, MPI_CHAR, final_maze.cells, sub_maze_size, MPI_CHAR, 0, MPI_COMM_WORLD);
    return final_maze; // Only rank 0 will have the final maze
}



int main(int argc, char** argv) {
// Print the Unicode character ⯃
    printf("Unicode character: ⯃\n");

    MPI_Init(&argc, &argv);
    int num_procs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int width = 0, height = 0;

    read_and_check_input(argc, argv, rank, num_procs, &width, &height);

    Maze sub_maze = generate_sub_maze(rank, num_procs, width, height);  // Each process

    Maze final_maze = merge_sub_mazes(sub_maze, rank, num_procs);   // Only rank 0 will have the final maze

    if (rank == 0) {
        print_maze(final_maze, false); // Print the final maze

    }
  
  




    // Maze building logic goes here

    //free the memory allocated for the final maze:
    if (rank == 0) free_maze(final_maze);
     
    MPI_Finalize();
    return 0;
}