#include "maze.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAZE_COLORS "\033[38;2;255;255;255;48;2;32;64;60m"
#define RESET_COLOR "\033[0m"
#define SAVE_CURSOR_POS "\0337"
#define RESTORE_CURSOR_POS "\0338"


// function that generates a ramdon result 0 or 1.
// result 1 with x (0..1) probability
bool ramdom_prob(float x) {
    return (rand() % 100) < x * 100;
}

bool is_inside_maze(Maze m, Coords c) {
    return c.x >= 0 && c.x < m.width && c.y >= 0 && c.y < m.height;
}

// whwther the cell is a path and inside the maze
bool is_path(Maze m, Coords c) {
    return is_inside_maze(m, c) && m.cells[c.y * m.width + c.x];
}

// Create a maze with the given dimensions
Maze create_maze(int width, int height) {
    Maze m;
    m.width = width;
    m.height = height;
    m.cells = (char*) malloc(width * height * sizeof(char));
    if (m.cells == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    return m;
}

// Free the memory allocated for the maze
void free_maze(Maze m) {
    free(m.cells);
}

// Fill the maze with random walls and paths
void fill_maze_random(Maze m,int rank) {
    //randomize with time seed
    srand(time(NULL)+rank);
    for (int i = 0; i < m.width * m.height; i++) {
        // path with 75% probability:
        m.cells[i] = ramdom_prob(0.7);
    }
}

/**
 * Print the maze to the console
 * if cursor_back is true, the cursor will be moved back to the top of the maze after printing
 */
void print_maze(Maze m, bool cursor_back) {
    if (cursor_back) printf(SAVE_CURSOR_POS);
    for (int i = 0; i < m.height; i++) {
        printf(MAZE_COLORS);
        for (int j = 0; j < m.width; j++) {
            printf ("%s", m.cells[i * m.width + j] ? " ":"╬");
        }
        printf(RESET_COLOR "\n");
    }
    printf("maze size: %d x %d\n", m.height, m.width);
    if (cursor_back) printf(RESTORE_CURSOR_POS);

}

//print a char at the given position
// it is suposed the cursor is at the top of the maze
// the cursor will be moved back to the top of the maze after printing
void print_char_in_maze(Maze m, Coords c, char ch) {
    printf("\033[%d;%dH", c.y + 1, c.x + 1);
    printf("%c", ch);
    printf("\033[%dA", m.height + 1);
}