#include "maze.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdarg.h>

#define MAZE_COLORS "\033[38;2;255;255;255;48;2;32;64;60m"
#define RESET_COLOR "\033[0m"
#define SAVE_CURSOR_POS "\0337"
#define RESTORE_CURSOR_POS "\0338"
#define CURSOR_RIGHT(n) "\033[" #n "C"


// Coords type:
bool Coords_equal(Coords a, Coords b) {
    return a.x == b.x && a.y == b.y;
}

// function that generates a ramdon result 0 or 1.
// result 1 with x (0..1) probability
bool ramdom_prob(float x) {
    return (rand() % 100) < x * 100;
}

bool is_inside_maze(Maze m, Coords c) {
    return c.x >= 0 && c.x < m.width && c.y >= 0 && c.y < m.height;
}

// whether the cell is a path and inside the maze
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

Coords maze_SE_corner(Maze m) {
    return (Coords){m.width - 1, m.height - 1};
}

Coords maze_NE_corner(Maze m) {
    return (Coords){m.width - 1, 0};
}
/**
 * Print the maze to the console
 * the cursor will be returned to the top of the maze after printing
 * 
 * NOTICE: each cell is printed with 2 spaces.
 * 
 */
void print_maze_r(Maze m) {
    printf(SAVE_CURSOR_POS);
    for (int i = 0; i < m.height; i++) {
        printf(MAZE_COLORS);
        for (int j = 0; j < m.width; j++) {
            printf ("%s", m.cells[i * m.width + j] ? "  ":"╬╬");
        }
        printf(RESET_COLOR "\n");
    }
    printf("maze size: %d x %d\n", m.height, m.width);
    printf(RESTORE_CURSOR_POS);

}

// Print the string after the maze and return the cursor to the top of the maze


void print_after_maze_r(Maze m, const char* format, ...) {
    printf(SAVE_CURSOR_POS);
    print_jump_maze(m, 0);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf(RESTORE_CURSOR_POS);
}

// moves the cursor to the next line below the maze
// extra_lines: number of extra lines to move the cursor
void print_jump_maze(Maze m,int extra_lines){
    printf("\033[%dB", m.height + extra_lines);
}


/**
 * Print a character at the given position in the maze
 * the cursor will be returned to the top of the maze after printing
 * It is supposed the cursor is at the top of the maze
 * 
 * NOTICE: each cell is printed with 2 spaces.
 * ch can be a string with 2 characters, e.g.: "  "
 *  or a unicode wide character, e.g.: "🐈"
 * 
 */
void print_char_in_maze_r(Maze m, Coords c, const char* ch) {
    printf(SAVE_CURSOR_POS);
    if (c.x > 0) printf("\033[%dC", c.x*2); // each cell is printed with 2 spaces
    if (c.y > 0) printf("\033[%dB", c.y);
    printf(MAZE_COLORS "%s" RESET_COLOR, ch);
    printf(RESTORE_CURSOR_POS);
}

// Corrige el laberinto según las reglas especificadas

// Corrige el laberinto según las restricciones
void correct_maze(Maze* m) {
    int width = m->width;
    int height = m->height;

    // Asegurar esquinas esenciales libres
    m->cells[0] = 1;                          // Esquina superior izquierda
    m->cells[width - 1] = 1;                  // Esquina superior derecha
    m->cells[height * width - 1] = 1;         // Esquina inferior derecha

    // Corregir filas para evitar celdas consecutivas con valor 0
    for (int i = 0; i < height; i++) {
        for (int j = 1; j < width; j++) {
            int idx = i * width + j;
            if (m->cells[idx] == 0 && m->cells[idx - 1] == 0) {
                m->cells[idx] = 1; // Cambiar celda actual a libre
            }
        }
    }

    // Asegurar que cada columna tenga al menos una celda libre
    for (int j = 0; j < width; j++) {
        int has_free_cell = 0;
        for (int i = 0; i < height; i++) {
            if (m->cells[i * width + j] == 1) {
                has_free_cell = 1;
                break;
            }
        }
        // Si no hay ninguna celda libre en la columna, libera una celda
        if (!has_free_cell) {
            int random_row = rand() % height;
            m->cells[random_row * width + j] = 1;
        }
    }
}