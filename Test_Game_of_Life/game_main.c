/*
    Enrique Lira Martinez
    A01023351
*/
#include "game_main.h"

board* get_boardFile(char* filename)
{
    int x = 0, y = 0;
    FILE* file = fopen(filename, "r");//read file name
    if (file == NULL)
    {
        printf("Cannot open image file!\n");// if it can be open send error message
        return 0;
    }
    // Scan size
    fscanf(file, "%i %i\n", &y, &x);//get the first values from file
    // Create board of that size
    board* board = create_board(x, y);//enter to a function that create the board
    // Read file into board
    int value = 0;
    for (int i = 0; i < y; i++)
    {
        for (int j = 0; j < x; j++)
        {
            fscanf(file, "%i", &value);//fill all the matrix
            if(value==1)
            {
              board->cells[i][j] = ALIVE;//the cell will be labeled with alive if the value is 1
            }
            else if (value==2)
            {
              board->cells[i][j] = ALIVE2;//the cell will be labeled with alive2 if the value is 2
            }
            else if (value==0)
            {
              board->cells[i][j] = DEAD;//the cell will be labeled with dead if the value is 0
            }
        }
    }
    fclose(file);//close file
    return board;
}

board* create_board(int x, int y)//function recives the size from the matrix
{
  board* board = malloc(sizeof(board));//allocate the matrix
  board->X = x;//get the sizes
  board->Y = y;
  allocate_board(board);//allocate the matrix
  return board;
}

void allocate_board(board* board)
{
  // Allocate the memory for INDEX array
  board->cells = malloc (board->Y * sizeof(int *));
  // Allocate the memory for all the DATA array
  board->cells[0] = calloc (board->Y * board->X, sizeof(int));
  // Add the rest of the pointers to the INDEX array
  for(int i = 0; i < (board->Y); i++)
  {
    // Add an offset from the beginning of the DATA array
    board->cells[i] = board->cells[0] + board->X * i;
  }
}

void delete_board(board* board)
{
  // Free the DATA array
    free(board->cells[0]);
    // Free the INDEX array
    free(board->cells);
      // Set the values for an empty matrix
    board->X = 0;
    board->Y = 0;
    board->cells = NULL;
    free(board);
}

/**
 * Counts live neighbours
 */
int count_Cells(board* board, int x, int y)//function to check neighbours in matrix of 3x3
{
  int neighbours = 0;
  for ( int i = -1; i < 2; i++ )
  {
    for ( int j = -1; j < 2; j++ )
    {
      if( i == 0 && j==0)
      {
        //if it is the center it will not be counted
      }
      else
      {
        if (x + i >= 0 && y + j >= 0 && y + j < board->Y && x + i < board->X)//detect the borders
        {
          if(board->cells[y + j][x + i] == ALIVE || board->cells[y + j][x + i] == ALIVE2  )//if the label is a alive cell
               neighbours++;//add 1 neighbour

        }
      }
    }
  }
  return neighbours;//return all neighbours alive
}
int countColor(board* board, int x, int y)//function to check colors in matrix of 3x3
{
    int average = 0;
    for ( int i = -1; i < 2; i++ )
    {
      for ( int j = -1; j < 2; j++ )
      {
        if( i == 0 && j==0)
        {
          //if it is the center it will not be counted
        }
        else
        {
          if (x + i >= 0 && y + j >= 0 && y + j < board->Y && x + i < board->X)//detect the borders
          {
            if(board->cells[y + j][x + i] == ALIVE)
                 average++;//add 1 if the value is 1
            if(board->cells[y + j][x + i] == ALIVE2)
                average=average+2;//add 2 if the value is 2
          }
        }
      }
    }
    return average;//return average of the color in neighbours
}
int next_state(board* board, int x, int y)//function to get the next label
{
    int status = board->cells[y][x];//detect the label
    int neighbours = count_Cells(board, x, y);//count the neighbours
    int average=countColor(board, x, y);//count the colors
    if (status == DEAD )//if the last state was dead
    {
        if (neighbours == 3)// it will be 3 to be alive
        {
            // Cell is now alive
            if(average < 5)//if there is less than 5 will be the first color else will be the second color
            {
              return ALIVE;
            }
            else
            {
              return ALIVE2;
            }
        }
        else
        {
            return status;// Cell state remains the same
        }
    }
    else
    {
        // If (less than 2) or (more than 3) neighbours alive it will be kill
        if (neighbours < 2 || neighbours > 3)
        {
            return DEAD;
        }
        else// If it is between 2 and 3 let it live
        {
          if(status == ALIVE)
          {
            return ALIVE;
          }
          else
          {
            return ALIVE2;
          }
        }
    }
}


void ThreadMode(board* last, board* previous)
{
    pthread_t* tid = malloc(MAX_THREADS * sizeof(pthread_t));//allocate thread
    int colums = (previous->X) / MAX_THREADS;// divides the colums into all the threads.

    if(colums != 0)
    {
        for(int t = 0; t < MAX_THREADS; t++)
        {
            make_thread(last, previous, &tid[t], t*colums, (t+1)*colums);//fill all thread with each part of the columns
        }
    }
    for(int y = 0; y < MAX_THREADS; y++)
    {
        pthread_join(tid[y], NULL);//get all the threads together
    }
    free(tid);
}
void make_thread(board* last, board* previous, pthread_t* tid, int min, int max)//fucntion to fill and create the thread
{
    thread* data = malloc(sizeof(thread));//allocate thread
    data->last = last;//fill with last board
    data->previous = previous;//fill with previous board
    data->min = min;//min in Y
    data->max = max;//max in Y
    pthread_create(tid, NULL, calculate_state, (void*)data);//create new thread
}

void* calculate_state(void* args)// function that calculates colums in a range
{
    thread* data = (thread*)args;//get the values for the thread

    for (int y = data->min; y < data->max; y++)//passthrought all the columns
    {
        for (int x = 0; x < (data->previous->X); x++)
        {
            data->previous->cells[y][x] = next_state(data->last, x, y);// get the next state
        }
    }
    free(data);//delete thread data
    pthread_exit(0);
}

// OPEN OMP MODE
void OMPMode(board* last, board* previous)
{
    int y, x;

    #pragma omp parallel for default(none) shared(last, previous) private(y, x)//parallel in wich it includes row col and the 2 last valueves from the for
    for (y = 0; y < previous->Y; y++)
    {
        for (x = 0; x < previous->X; x++)
        {
            previous->cells[y][x] = next_state(last, x, y);//gives the next status from each cell
        }
    }
}
void NormalMode(board* last, board* previous)//linear option for the function
{
    int y, x;
    for (y = 0; y < previous->Y; y++)
    {
        for (x = 0; x < previous->X; x++)
        {
            previous->cells[y][x] = next_state(last, x, y);//gives the next status from each cell
        }
    }
}
