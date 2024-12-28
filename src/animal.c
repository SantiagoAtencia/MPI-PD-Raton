#include "maze.h"
#include "animal.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <mpi.h>

#define PROBABILITY_SAME_DIRECTION 0.9



/**
 * Calculate the next coordinate, 1 step towards direction
 * Does not check anything.
 *
 */
Coords cell_step_towards(Coords cell, Direction dir){
    switch (dir){
        case UP:
            return (Coords){cell.x, cell.y-1};
        case UP_RIGHT:
            return (Coords){cell.x+1, cell.y-1};
        case RIGHT:
            return (Coords){cell.x+1, cell.y};
        case DOWN_RIGHT:
            return (Coords){cell.x+1, cell.y+1};
        case DOWN:
            return (Coords){cell.x, cell.y+1};
        case DOWN_LEFT:
            return (Coords){cell.x-1, cell.y+1};
        case LEFT:
            return (Coords){cell.x-1, cell.y};
        case UP_LEFT:
            return (Coords){cell.x-1, cell.y-1};
    }
    return cell;

}

/**
 * Randomly choose a direction, 
 * with a probability of PROBABILITY_SAME_DIRECTION to keep the same direction
 */
Direction ramdom_dir_from(Direction last_dir){
    // generate a random number between 0 and 1
    int r = rand() % (256*256);
    if (r < PROBABILITY_SAME_DIRECTION * 256*256){
        return last_dir;
    }
    else{
        return (Direction) (rand() % 8);
    } 
}

/**
 * Fix the direction if it is not valid
 * 
 * @param dir: the direction to fix
 * @param animal: the animal that will move
 * @param maze: the maze
 * 
 * @return the fixed direction
 * 
 * If the direction is not valid, it will try the next direction
 * If none of the 8 directions are valid, it will halt the program
 * 
 */
Direction fix_dir(Direction dir, Animal* animal, Maze maze){
    // repeat until the direction is valid:
    //  calculate the new position
    //  check if the new position is valid
    //  if not, increment the direction
    //  if none of the 8 directions are valid, halt the program: error
    Coords new_pos;
    for (int i = 0; i < 8; i++){
        new_pos = cell_step_towards(animal->pos, dir);
        if (is_path(maze, new_pos)) return dir;
        dir = (Direction) ((dir + 1) % 8);  // try the next direction  
    }
    printf("Error: no valid direction found\n");
    exit(1); // halt the program
        // if that happens, it means that the animal is stuck in a dead end
}


/** 
 * Randomly move the animal
 * @param a: the animal to move
 * @param m: the maze
 * 
 * The current direction has more probability to be chosen 
 * 
 */
void move_animal_randomly(Animal* animal, Maze maze){
    // choose random direction, based on the current direction:
    Direction new_dir= ramdom_dir_from(animal->last_dir);
    new_dir= fix_dir(new_dir,animal,maze);// fix the direction if it is not valid
    animal->pos = cell_step_towards(animal->pos, new_dir); // move the animal
};


//mouse routine:
// receive the maze
// initialize the position of the mouse
// loop:
//    wait for a while, constant time
//    check if a there is a message from the game routine
//      if the message is to stop: break the loop
//    update the position of the mouse
//    send the new position to the game routine

//cat routine: idem.

/**
 * The animal routine
 * 
 * @param animal: the animal to move, in its initial position
 * @param maze: the maze
 * 
 * The animal will move randomly in the maze
 * Must be called only in a rank
 * 
 */
void animal_routine(Animal* p_animal, int width, int height){
    
    //receive the maze:
    //create a maze local, to copy from rank 0:
    Maze maze = create_maze(width, height);

    //receive from rank 0, mpi:
    MPI_Recv(maze.cells, width*height, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //loop:
    while (true){
        uleep(500000);//wait for a while, constant time
        //check if a there is a message from the game routine mpi:
        int flag;
        MPI_Status status;
        MPI_Iprobe(0, 0, MPI_COMM_WORLD, &flag, &status);
        if (flag){
            //just break the loop, because is the only message that can be received
            break;
        }

        //update the position of the mouse
        move_animal_randomly(p_animal, maze);
        //send the new position to the game routine
        MPI_Send(&p_animal->pos, 1, MPI_2INT, 0, 0, MPI_COMM_WORLD);
    }
    //free the local maze copy
    free_maze(maze);

}