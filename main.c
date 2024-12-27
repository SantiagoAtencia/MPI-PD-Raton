#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN_DIM 4
#define MAX_DIM 100

// maze type: an matrix of booleans (0 or 1) (0 = wall, 1 = path) for efficiency 1 byte per cell
typedef struct maze {
    int width;
    int height;
    char* cells; // 2D array of cells rows,cols
} maze;

//🐁🐁🐁🐁🐁🐭🐭🐭🐈🐈🐱🐱🕱🕱☠☠
//╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬
//╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬🐈╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬
//xxxxxxxxxxxxxxxx🐈xxxx╬╬    ╬╬╬╬╬╬╬  ╬╬╬╬╬╬
// ██████
// █    █
// ████  ▓▓▓
// █   ▓█ ▓▓▓▓ █
// ███▓▓▓▓ ▓▓███▓▓▓▓▓▓



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
        printf("Building a maze of dimensions %d x %d with %d processes.\n", *width, *height, num_procs);
    }

    MPI_Bcast(&valid_input, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (!valid_input) {
        MPI_Finalize();
        exit(1);
    }

    MPI_Bcast(width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(height, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

/**
 * Generates a sub-maze for each process.
 * 
 * Each process generates a sub-maze of dimensions width x height/num_procs.
 */
maze generate_sub_maze(int rank, int num_procs, int width, int height) {
    //allocate memory for the sub-maze
    int sub_height = height / num_procs;
    char* sub_maze = (char*) malloc(width * sub_height * sizeof(char));
    
    //fill the sub-maze with walls and paths randomly
    srand(rank+2); // seed the random number generator
    for (int i = 0; i < width * sub_height; i++) {
        sub_maze[i] = rand() % 2;
    }
 // Create and return the maze struct
    maze m;
    m.width = width;
    m.height = sub_height;
    m.cells = sub_maze;
    return m;
}

/**
 * Merges the sub-mazes into the final maze.
 * 
 */
maze merge_sub_mazes(maze sub_maze, int rank, int num_procs) {
    int sub_maze_size = sub_maze.width * sub_maze.height;
    // allocate memory for the final maze in rank 0
    char* final_maze_buffer;

    if (rank == 0) {
        final_maze_buffer = (char*) malloc(sub_maze_size * num_procs * sizeof(char));
    }
    // gather all sub-mazes into the final maze
    MPI_Gather(sub_maze.cells, sub_maze_size, MPI_CHAR, final_maze_buffer, sub_maze_size, MPI_CHAR, 0, MPI_COMM_WORLD);
    // Create and return the final maze struct for rank 0
    maze m;
    if (rank == 0) {
        m.width = sub_maze.width;
        m.height = sub_maze.height * num_procs;
        m.cells = final_maze_buffer;
    }
    return m;
}

// print the maze by rank 0
// with a parameter that specifies if the next time will be printed in the same line or not
// Remembers the mode of the last print and the data needed to print in the terminal overwriting the last print
void print_maze(maze m, int rank, int overwrite) {
    if (rank != 0) return;
    
    static int last_height = 0;

    if (overwrite) {
        printf("\033[%dA", last_height + 1); // Move cursor up
    }

    // Set background color to dark green and foreground color to light gray
    printf("\033[48;5;22m"); // Background color: dark green
    printf("\033[38;5;250m"); // Foreground color: light gray

    for (int i = 0; i < m.height; i++) {
        for (int j = 0; j < m.width; j++) {
            printf("\033[48;5;20m"); // Background color: dark green
            printf("\033[38;5;250m"); // Foreground color: light gray
            printf("%s", m.cells[i * m.width + j] ? " " : "╬");
            // Reset colors
            printf("\033[0m");}
        printf("\n");
    }
    

    last_height = m.height;

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

    maze sub_maze = generate_sub_maze(rank, num_procs, width, height);

    maze final_maze = merge_sub_mazes(sub_maze, rank, num_procs);   // Only rank 0 will have the final maze

    print_maze(final_maze, rank, 0); // Print the final maze
    printf("\n");
    print_maze(final_maze, rank, 0); // Print the final maze
    printf("\n");
    print_maze(final_maze, rank, 1); // Print the final maze
    printf("\n");




    // Maze building logic goes here

    MPI_Finalize();
    return 0;
}