#ifndef MAZE_H
#define MAZE_H

#include <stdbool.h>

#define MIN_DIM 4
#define MAX_DIM 100

// Maze type: a matrix of booleans (0 or 1) (0 = wall, 1 = path) for efficiency 1 byte per cell
typedef struct Maze {
    int width;
    int height;
    char* cells; // Contiguous memory for 2D array
} Maze;

// Coords type:
typedef struct Coords {
    int x;
    int y;
} Coords;

// Function declarations
Maze create_maze(int width, int height);
void free_maze(Maze m);
void fill_maze_random(Maze m,int rank);

void print_maze(Maze m);
void print_jump_maze(Maze m);
void print_char_in_maze(Maze m, Coords c, const char* ch);

/**
 *  7 0 1
 *   ↖↑↗
 *  6← →2 
 *   ↙↓↘
 *  5 4 3  
 * 
 * 
 */


#endif // MAZE_H
