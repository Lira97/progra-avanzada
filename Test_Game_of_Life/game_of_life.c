/*
    Enrique Lira Martinez
    A01023351

    Simulation of Conway's Game of Life using OpenMP
    Based on the explanations at:
        https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
    Online versions:
        http://pmav.eu/stuff/javascript-game-of-life-v3.1.1/
        https://bitstorm.org/gameoflife/

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include "pgm_image.h"
//#include "ppms.h"
// ADD YOUR EXTRA LIBRARIES HERE
#include "game_main.h"

#define STRING_SIZE 50

///// Function declarations /////
void usage(const char * program);
void lifeSimulation(int iterations, char * start_file,int thread_option);
void saveAsPPM(const board * board, int iteration);
// ADD YOUR FUNCTION DECLARATIONS HERE

int main(int argc, char * argv[])
{
    char * start_file = "Boards/bi_pulsar.txt";
    int iterations = 5;
    int thread_option = 0;
    printf("\n=== GAME OF LIFE ===\n");
    printf("{By: Enrique Lira Martinez\n");

    // Parsing the arguments
    if (argc == 2)
    {
        iterations = atoi(argv[1]);
    }
    else if (argc == 3)
    {
        iterations = atoi(argv[1]);
        start_file = argv[2];
    }
    else if (argc == 4)
    {
        iterations = atoi(argv[1]);
        start_file = argv[2];
        thread_option = atoi(argv[3]);//define the parallelism you wish
    }
    else if (argc != 1)
    {
        usage(argv[0]);
        return 1;
    }

    // Run the simulation with the iterations specified
    printf("Running the simulation with file '%s' using %d iterations\n", start_file, iterations);
    lifeSimulation(iterations, start_file,thread_option);

    return 0;
}

// Print usage information for the program
void usage(const char * program)
{
    printf("Usage:\n");
    printf("%s [iterations] [board file]\n", program);
}
clock_t t;
// Main loop for the simulation
void lifeSimulation(int iterations, char * start_file,int thread_option)
{
  board* last = get_boardFile(start_file);
  printf("Board ready! (%i)\n", iterations);
  double avg_png_time = 0;
  double avg_sim_time = 0;
  double start = omp_get_wtime();
  if(thread_option == 1)
    printf("OMP MODE \n");
  else if(thread_option == 2)
    printf("THREAD MODE \n");
  else
    printf("NORMAL MODE \n");
  for (int i = 0; i < iterations; i++)
  {
      saveAsPPM(last,i);// write PPM File

      avg_png_time += (double)(clock()-t)/CLOCKS_PER_SEC;//start time for thread


      board* current = create_board(last->X, last->Y);// Create new board

      t = clock();
      if(thread_option == 1)
        OMPMode(last, current);//OMP option
      else if(thread_option == 2)
      ThreadMode(last, current);//thread option
      else
      NormalMode(last, current);//linear option

      avg_sim_time += (double)(clock()-t)/CLOCKS_PER_SEC;

      delete_board(last);  // free last board

      last = current;  // Make current board the last

  }
  delete_board(last);//free the allocate board
printf("Time: \t %f \n", omp_get_wtime()-start);
//system("ffmpeg -pattern_type glob -framerate 25 -i 'Images/*.ppm output.mp4");
}

// Save the contents of the board as a PGM image
void saveAsPPM(const board * board, int iteration)
{
  pgm_t board_image;// create a new image to print the board
  char name[50];//char to keep the name
  sprintf(name, "Images/image_%d.ppm", iteration);//gives the name to the new file
  strcpy(board_image.magic_number, "P3");//write the first line of the file
  board_image.image.width = board->X;//write the size of the matrix
  board_image.image.height = board->Y;//write the size of the matrix
  board_image.max_value = 255;//write the max value of each pixel
  int red,green,blue = 0;// create variables for each pixel
  allocateImage( &board_image.image );//allocate image in memory
  for (int i=0; i<board_image.image.height; i++)// loop to fill the image
  {
      for (int j=0; j<board_image.image.width; j++)
      {
      switch (board->cells[i][j])//switch for each case in the board
      {
          case DEAD://white color
              red = 255;
              green = 255;
              blue = 255;
              break;
          case ALIVE2://red color
              red = 255;
              green = 51;
              blue = 51;
              break;
          case ALIVE://black color
              red = 0;
              green = 0;
              blue = 0;
              break;
      }
      //give the color to the pixels in the image
      board_image.image.pixels[i][j].value[R] =  red;
      board_image.image.pixels[i][j].value[G] =  green;
      board_image.image.pixels[i][j].value[B] =  blue;
  }
}
writePGMFile(name, &board_image);//function to write the image
freeImage( &board_image.image);//free the image allocate

}
