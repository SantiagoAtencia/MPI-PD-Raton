#include "maze_corrector.h"
#include <stdlib.h>
#include <string.h>
#include "maze.h"
// Corrige el laberinto según las reglas especificadas


// Corrige el laberinto según las restricciones
void correct_maze(Maze* m) {
    int width = m->width;
    int height = m->height;

    // Asegurar esquinas libres
    m->cells[0] = 1;                          // Esquina superior izquierda
    m->cells[width - 1] = 1;                  // Esquina superior derecha
    m->cells[(height - 1) * width] = 1;       // Esquina inferior izquierda
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