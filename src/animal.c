#include "maze.h"
#include "animal.h"
#include <stdlib.h>
#include <unistd.h> // For usleep
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <mpi.h>


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
 *
 * @return the fixed direction
 * 
 * If the direction is not valid, it will try the next direction
 * If none of the 8 directions are valid, it will halt the program
 * 
 */
Direction _animal_fix_dir(Animal animal, Direction dir){
    // repeat until the direction is valid:
    //  calculate the new position
    //  check if the new position is valid
    //  if not, increment the direction
    //  if none of the 8 directions are valid, halt the program: error
    Coords new_pos;
    for (int i = 0; i < 8; i++){
        new_pos = cell_step_towards(animal.pos, dir);
        if (is_path(animal.maze, new_pos)) return dir;
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
void move_animal_randomly(Animal* p_animal){
    // print spaces to erase the previous position
    //print_char_in_maze_r(p_animal->maze, p_animal->pos, "  ");

    // choose random direction, based on the current direction:
    Direction new_dir= ramdom_dir_from(p_animal->last_dir);
    new_dir= _animal_fix_dir(*p_animal,new_dir);// fix the direction if it is not valid
    p_animal->pos = cell_step_towards(p_animal->pos, new_dir); // move the animal

    // print the new position
    //print_char_in_maze_r(p_animal->maze, p_animal->pos, p_animal->icon);
    print_debug("Animal %d moved to (%d,%d)\n", p_animal->id, p_animal->pos.x, p_animal->pos.y);
};



/**
 * The animal routine
 * 
 * @param animal: the animal to move, in its initial position
 * @param maze: the maze
 * 
 * The animal will move randomly in the maze
 * Must be called only in a rank
 * Requires the animal.maze to be with the correct dimensions,
 *  but the cells unallocated, because the maze will be received from rank 0
 *  and allocated in this function
 * 
 */
void animal_routine(Animal* p_animal){
    MPI_Status status;
    int flag;
    
    //receive the maze:
    //re-create a maze local, to copy from rank 0.
    p_animal->maze = create_maze(p_animal->maze.width, p_animal->maze.height); //allocate the cells

    //receive from rank 0, mpi:
    MPI_Recv(p_animal->maze.cells, p_animal->maze.width * p_animal->maze.height, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //loop:
    while (true){
        usleep(p_animal->time_to_sleep*1000); // wait for a while, constant time
        //check if a there is a message from the game routine mpi
        
        MPI_Iprobe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &flag,  MPI_STATUS_IGNORE);
        if (flag){
            print_debug("Animal %d received a message from game routine\n", p_animal->id);
            break; //just break the loop, because is the only message that can be received
        }

        //update the position of the mouse
        move_animal_randomly(p_animal);
        //send the new position to the game routine
        MPI_Send(&p_animal->pos, 1, MPI_2INT, 0, 0, MPI_COMM_WORLD);
    }
    //free the local maze copy
    free_maze(p_animal->maze);

}