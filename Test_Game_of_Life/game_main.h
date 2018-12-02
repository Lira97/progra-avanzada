/*
    Enrique Lira Martinez
    A01023351
*/
#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <omp.h>
#define MAX_THREADS 4
// Board struct
typedef struct board_struct {
    int X;
    int Y;
    int** cells;
} board;
/**
 * Struct for passing data to threads
 */
typedef struct thread_data{
    board* last;
    board* previous;
    int min;
    int max;
}thread;
// Cell status enum
typedef enum cell_status {
    DEAD,
    ALIVE,
    ALIVE2
} cs_t;

// Create and allocate a board
board* create_board(int sizex, int sizey);
// Destroy and free all data in a board
void delete_board(board* board);

// Create a board from a filename
board* get_boardFile(char* filename);
int next_state(board* board, int x, int y);
int count_Cells(board* board, int x, int y);
int count_Color(board* board, int x, int y);
void allocate_board(board* board);
// Main simulation function, has different implementations
void ThreadMode(board* last, board* previous);
void NormalMode(board* last, board* previous);
void OMPMode(board* last, board* previous);
void make_thread(board* last, board* current, pthread_t* tid, int y_min, int y_max);
void* calculate_state(void* args);
#endif
