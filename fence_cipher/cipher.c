/*
  Enrique Lira Martinez
  A01023351
  Fence rail cipher

      To run this program
      you should compile like this : "gcc readfile.c main.c cipher.c -o main" or you can run the makefile
      and exceute like this: "./main"

      then the program will create a file with the encode or decode message it dependes on the user
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cipher.h"

void allocImage(matrix_t* matrix)
{
    matrix->railMatrix = (char**)malloc((matrix->height) * sizeof(char*));//the matrix is assigned in the memory

     for(int i = 0; i < matrix->height; i++)
     {
         matrix->railMatrix[i] = (char*)calloc((matrix->width),sizeof(char));//it allocate all the rows by the number of columns
     }
}
void freeMemory(matrix_t* matrix)
{

    free(matrix->railMatrix);// the image in the struct is delete
    free(matrix);//as well as the entire struct
}

char * decrypt(char * text, int key)
{
  char * result;//char for the result
  int k = -1, row = 0, col = 0, m = 0;
  matrix_t * encodedMatrix = malloc(sizeof(matrix_t));//create a char to return the result
  encodedMatrix->width=strlen(text);//size for the matrix
  encodedMatrix->height=key;//allocate the matrix in memory
  allocImage(encodedMatrix);;//allocate the matrix in memory
  result = calloc((encodedMatrix->width),sizeof(char));//allocate the char result in memory

    for(int i = 0; i < key; ++i)
    {
        for(int j = 0; j < encodedMatrix->width; ++j)
        {
            encodedMatrix->railMatrix[i][j] = '\n';//fill all the matrix with enters
        }
    }

      for(int i = 0; i < encodedMatrix->width; ++i)
      {
          if(row == 0 || row == key-1)//if the matrix get to the peck of the limit it will go up or down
          {
              k= k * (-1);
          }
          encodedMatrix->railMatrix[row][col++] = '*';//left a path to get the wave

          row = row + k;
      }

      for(int i = 0; i < key; ++i)
      {
          for(int j = 0; j < encodedMatrix->width; ++j)
          {
                if(encodedMatrix->railMatrix[i][j] == '*')//follow the path
                {
                  encodedMatrix->railMatrix[i][j] = text[m++];//fill with the string
                }
              }
            }

      row = 0;
      col = 0;
      k = -1;
      for(int i = 0; i < encodedMatrix->width; ++i)//start to read it again
      {
        result[i] = encodedMatrix->railMatrix[row][col++];//fill the char with the wave
          if(row == 0 || row == key-1)
              k= k * (-1);
          row = row + k;
      }
  freeMemory(encodedMatrix);//it frees the memory
  return result;//return the string
}

char * encode(char * text, int key)
{
  char * result;//create a char to return the result
  matrix_t * encodedMatrix = malloc(sizeof(matrix_t));//create a matrix for the wave method
  encodedMatrix->width=strlen(text);//size for the matrix
  encodedMatrix->height=key;//size for the matrix
  allocImage(encodedMatrix);//allocate the matrix in memory
  result = calloc((encodedMatrix->width),sizeof(char));//allocate the char result in memory

  int k = -1;
  int row = 0;
  int col = 0;
  for(int i = 0; i < key; ++i)
  {
    for(int j = 0; j < encodedMatrix->width; ++j)
    {
      encodedMatrix->railMatrix[i][j] = '\n';//fill all the matrix with enters
    }
  }

    for(int i = 0; i < encodedMatrix->width; ++i)
    {
        encodedMatrix->railMatrix[row][col++] = text[i];//put the sting in the wave matrix

        if(row == 0 || row == key-1)//if the matrix get to the peck of the limit it will go up or down
          {
            k = k * (-1);
          }
        row = row + k;
    }
    int count = 0;//count for the resulting char
    for(int i = 0; i < key; ++i)
    {
      for(int j = 0; j < encodedMatrix->width; ++j)
      {
        if(encodedMatrix->railMatrix[i][j] != '\n')//if there is no enter
        {
          result[count] = encodedMatrix->railMatrix[i][j];//fill the char with the reading matrix
          count++;
        }
      }
    }
  freeMemory(encodedMatrix);//it frees the memory
  return result;//return the result
}
