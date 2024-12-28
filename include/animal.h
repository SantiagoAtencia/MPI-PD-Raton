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

/**
 * Randomly choose a direction, 
 * with a probability of PROBABILITY_SAME_DIRECTION to keep the same direction
 */
#define PROBABILITY_SAME_DIRECTION 0.9
Direction ramdom_dir_from(Direction last_dir);

// calculate the next coordinate, 1 step towrds direction
// Does not check anythig.
Coords cell_step_towards(Coords cell, Direction dir);




// Animal "class":
typedef struct Animal {
    int id;
    Coords pos;
    Direction last_dir;
    Maze maze;          // The maze where the animal is
    char* icon;         // The icon to print in the maze
    int time_to_sleep;  // Time to sleep in milliseconds
} Animal;

//Randomly move the animal
void move_animal_randomly(Animal* p_animal);

//Animal routine, starts moving the animal
// communication with the game routine at rank 0
void animal_routine(Animal* p_animal);


#endif // ANIMAL_H