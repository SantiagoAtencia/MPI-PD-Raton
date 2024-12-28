#ifndef ANIMAL_H
#define ANIMAL_H

#include <stdbool.h>
#include "maze.h" // Include the header file where Coord is defined


/**
 *  7 0 1
 *   ↖↑↗
 *  6← →2 
 *   ↙↓↘
 *  5 4 3  
 * 
 * 
 */
// Direction type:
typedef enum Direction {
    UP = 0, UP_RIGHT = 1, RIGHT = 2, DOWN_RIGHT = 3,
    DOWN = 4, DOWN_LEFT = 5, LEFT = 6, UP_LEFT = 7
} Direction;



// Animal type:
typedef struct Animal {
    int id;
    Coords pos;
    Direction last_dir;
} Animal;

//Randomly move the animal
void move_animal_randomly(Animal* animal, Maze maze);

// calculate the next coordinate, 1 step towrds direction
// Does not check anythig.
Coords cell_step_towards(Coords cell, Direction dir);


#endif // ANIMAL_H