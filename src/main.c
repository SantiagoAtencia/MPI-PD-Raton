#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "maze.h"
#include "animal.h"

#define MIN_DIM 4
#define MAX_DIM 100


//☠🐁🐁🐁🐁🐭🐭🐭🐭🐈🐈🐱🐱🕱🕱☠
//╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬
//╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬🐈╬╬╬╬🐁╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬
//xxxxxxxxxxxxxxxx🐈xxxx╬╬    ╬╬╬╬╬╬╬  ╬╬╬╬╬╬
// ██████
// █    █
// ████  ▓▓▓
// █   ▓█ ▓▓▓▓🐁 █🐁
// ███▓▓▓▓ ▓ ▓ █ █ █ ▓ ▓▓▓▓▓
//🐁🐁🐁🐁🐁🐭🐭🐭🐈🐈🐱🐱🕱🕱☠



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
    // the other processes will return an empty maze, but correct dimensions
    final_maze.width = sub_maze.width;
    final_maze.height = sub_maze.height * num_procs;
    return final_maze; // Only rank 0 will have the final maze
}

// Game routine:
// call only in rank 0
void game_routine(Maze maze){
    // Send the maze to the  mouse and cat, with mpi:
    MPI_Send(maze.cells, maze.width * maze.height, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
    MPI_Send(maze.cells, maze.width * maze.height, MPI_CHAR, 2, 0, MPI_COMM_WORLD);
    // start a "timer"
    int time_left = 10; //seconds
    int start_time = time(NULL); //current time
    Coords mouse_pos={0,0};
    Coords cat_pos=maze_SE_corner(maze);

    MPI_Status status;
    Coords received_pos;

    do{
        // wait for message from any, mouse or cat
        MPI_Recv(&received_pos, 1, MPI_2INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        if (status.MPI_SOURCE == 1){
            mouse_pos = received_pos;
        } else if (status.MPI_SOURCE == 2){
            cat_pos = received_pos;
        }
        //check if the mouse is in the same position as the cat
        if (mouse_pos.x == cat_pos.x && mouse_pos.y == cat_pos.y){
            printf("The cat caught the mouse!\n");
            break;
        }


  
    }
    // loop:
    //    waitfor messagw from mouse or cat
    //    if message from mouse: update position of mouse
    //    if message from cat: idem

    //   check if the mouse is in the same position as the cat
    //    check if the mouse is at goal cell.
    //    check if time is up

    //    if any of the above conditions is true:
    //        send a message to the cat and mouse to stop
    //        print the winner
    //        break the loop
    //    else: print the time left
}

// Send the maze to the  mouse and cat
// start a "timer"
// loop:
//    waitfor messagw from mouse or cat
//    if message from mouse: update position of mouse
//     print the mouse (erase previous position and print the new position)
//    if message from cat: idem

///   check if the mouse is in the same position as the cat
//    check if th mouse is at goal cell.
//    check if time is up

//    if any of the above conditions is true:
//        send a message to the cat and mouse to stop
//        print the winner
//        break the loop
//    else: print the time left


// pass a unallocated maze to the function, but with correct dimensions
// call only in a single rank, not 0
void mouse_routine(Maze maze){
    Animal mouse;
    mouse.pos = (Coords){0, 0};
    mouse.icon = "🐁";
    mouse.time_to_sleep = 1000; //ms
    mouse.maze=maze;
    mouse.id=1;
    animal_routine(&mouse);
}

// pass a unallocated maze to the function, but with correct dimensions
// call only in a single rank, not 0
void cat_routine(Maze maze){
    Animal cat;
    cat.pos = maze_SE_corner(maze);
    cat.icon = "🐈";
    cat.time_to_sleep = 1000; //ms
    cat.maze=maze;
    cat.id=2;
    animal_routine(&cat);
}

////////////////////////////////////////////////////////////////
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
        print_maze(final_maze); // Print the final maze

        print_char_in_maze(final_maze, (Coords){0, 0}, "🐈"); // Print the start point
        print_char_in_maze(final_maze, (Coords){22-1,8-1}, "🐈"); // Print the end point
        print_char_in_maze(final_maze, (Coords){22-1,8-1}, "  "); // Print the end point
        
        print_jump_maze(final_maze);
    }
  
    if (rank == 0) game_routine(final_maze);
    if (rank == 1) mouse_routine(final_maze);
    if (rank == 2) cat_routine(final_maze);


    //free the memory allocated for the final maze:
    if (rank == 0) free_maze(final_maze);
     
    MPI_Finalize();
    return 0;
}