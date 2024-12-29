#ifndef MAZE_H
#define MAZE_H

#include <stdbool.h>

#define MIN_DIM 4
#define MAX_DIM 100

#define MAZE_COLORS "\033[38;2;255;255;255;48;2;32;64;60m"
#define RESET_COLOR "\033[0m"
#define SAVE_CURSOR_POS "\0337"
#define RESTORE_CURSOR_POS "\0338"
#define CURSOR_RIGHT(n) "\033[" #n "C"

// Coords type:
typedef struct Coords {
    int x;
    int y;
} Coords;

bool Coords_equal(Coords a, Coords b);

// Maze type: a matrix of booleans (0 or 1) (0 = wall, 1 = path) for efficiency 1 byte per cell
typedef struct Maze {
    int width;
    int height;
    char* cells; // Contiguous memory for 2D array
} Maze;



// Function declarations
Maze create_maze(int width, int height);        //allocates memory
void free_maze(Maze m);                         //frees memory
void fill_maze_random(Maze m,int rank);
Coords maze_SE_corner(Maze m);
Coords maze_NE_corner(Maze m);
bool is_path(Maze m, Coords c);

void print_maze_r(Maze m);                          //prints the maze and return cursor
void print_after_maze_r(Maze m, const char* format, ...);    //prints the string after the maze and return cursor
void print_jump_maze(Maze m, int extre_lines);      //jump to extre_lines after the maze
void print_char_in_maze_r(Maze m, Coords c, const char* ch); //print a character in the maze and return cursor

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
