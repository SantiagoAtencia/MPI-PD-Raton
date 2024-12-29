#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include "maze.h"
#include "animal.h"
#include <time.h>


#define MIN_DIM 4
#define MAX_DIM 100
#define DEBUG 0

int num_procs, rank; //global

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

void print_debug(const char* format, ...) {
    if (DEBUG) {
        printf(" ---- DEBUG: [rank %d]: ", rank);
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        
    }
}

void print_usage() {
    printf("Usage: mpirun -np <num_processes> ./maze_race <width> <height>\n");
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
    print_debug("Maze sent to mouse and cat\n");
    // start a "timer"
    int start_time = time(NULL); //current time
    int end_time = start_time + 10; //10 seconds
   
    Coords mouse_pos={0,0};
    Coords cat_pos=maze_SE_corner(maze);

    MPI_Status status;
    Coords received_pos;
    char* end_cause;

    do{
        // wait for message from any, mouse or cat
        print_debug("Waiting for message from mouse or cat\n");
        MPI_Recv(&received_pos, 1, MPI_2INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        if (status.MPI_SOURCE == 1){
            print_char_in_maze_r(maze, mouse_pos, "  "); //erase the previous position
            print_char_in_maze_r(maze, received_pos, "🐁"); //print the new position
            mouse_pos = received_pos;
            print_debug("Mouse position received (%d,%d)\n", mouse_pos.x, mouse_pos.y);
        } else if (status.MPI_SOURCE == 2){
            print_char_in_maze_r(maze, cat_pos, "  "); //erase the previous position
            print_char_in_maze_r(maze, received_pos, "🐈"); //print the new position
            cat_pos = received_pos;
            print_debug("Cat position received (%d,%d)\n", cat_pos.x, cat_pos.y);
        }
        //check if the mouse is in the same position as the cat
        if (Coords_equal(mouse_pos, cat_pos)){
            end_cause = "The cat caught the mouse!";
            break;
        }
        //check if the mouse is at goal cell.
        if (Coords_equal(mouse_pos, maze_NE_corner(maze))){
            end_cause = "The mouse reached the goal!";
            break;
        }
        //check if time is up
        if (time(NULL) >= end_time){
            end_cause = "Time is up!";
            break;
        }
        //print the time left TODO: print the time left
        
        print_after_maze_r(maze, CURSOR_RIGHT(20) "mouse:(%d,%d) cat:(%d,%d) Time left: %d s  ", 
             mouse_pos.x, mouse_pos.y,
             cat_pos.x, cat_pos.y,
             end_time - time(NULL));
        print_debug("Time left: %d\n", end_time - time(NULL));       
    } while (true);
    //notify the mouse and cat that the game is over, just a flag:
    MPI_Send(&received_pos, 1, MPI_2INT, 1, 0, MPI_COMM_WORLD);
    MPI_Send(&received_pos, 1, MPI_2INT, 2, 0, MPI_COMM_WORLD);
    

    print_jump_maze(maze, 2);
    printf("Game over! %s\n", end_cause);
}




// pass a unallocated maze to the function, but with correct dimensions
// call only in a single rank, not 0
void mouse_routine(Maze maze){
    Animal mouse;
    mouse.pos = (Coords){0, 0};
    mouse.icon = "🐁";
    mouse.time_to_sleep = 200; //ms
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
    cat.time_to_sleep = 100; //ms
    cat.maze=maze;
    cat.id=2;
    animal_routine(&cat);
}

////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {

    setvbuf(stdout, NULL, _IONBF, 0); // Set stdout to no buffering
    
    MPI_Init(&argc, &argv);
    
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int width = 0, height = 0;

    read_and_check_input(argc, argv, rank, num_procs, &width, &height);

    Maze sub_maze = generate_sub_maze(rank, num_procs, width, height);  // Each process

    Maze final_maze = merge_sub_mazes(sub_maze, rank, num_procs);   // Only rank 0 will have the final maze

    if (rank == 0) {
        print_maze_r(final_maze); // Print the final maze

        if (DEBUG) print_jump_maze(final_maze, 2);

    }
  
    print_debug("Maze generated\n");
    if (rank == 0) game_routine(final_maze);
    if (rank == 1) mouse_routine(final_maze);
    if (rank == 2) cat_routine(final_maze);


    //free the memory allocated for the final maze:
    if (rank == 0) free_maze(final_maze);

    // barrier to wait for all processes to finish
    print_debug("at Barrier\n");
    MPI_Barrier(MPI_COMM_WORLD); 
    MPI_Finalize();
    return 0;
}