/*
	Enrique Lira Martinez
	A01023351
	Matrix multiplication
  How to use:

      To run this program
      you should compile like this : "gcc matrix_multiplication.c main.c -o matrix_multiplication"
      and exceute like this: "./program Matrix1.txt Matrix2.txt"

      then the program will create a file called result.txt that cointains the result matrix

*/
#include "matrix_multiplication.h"

void freeMemory(Matrix* matrix)
{
    for(int i = 0; i < matrix->height; i++)//this for pass to all the columns in the matrix
    {
        free ((matrix->Mtx)[i]);// it deletes all the rows by the number of columns
    }
    free (matrix->Mtx);// the matrix in the struct is delete
    free (matrix);//as well as the entire struct
}

Matrix* readFile(char* NameFile)//function recives a string with the name of the file
{
   FILE* file = fopen(NameFile, "r");//this variables is meant to open the file

   Matrix * matrix;//a new struct is created to be used for the matrix in the file
   matrix = (Matrix*)malloc(sizeof (Matrix));//the struct is allocate in memory

   char buffer[255];//this varible is to store the string of each line
   char*nextChar;// this variable is for each letter of the file

   fgets(buffer, sizeof buffer, file);//it gets the fisrt line of the file

   matrix->height = strtol(buffer, &nextChar, 10);//with the strtol function we can read letter by letter and transform it into an long integer
   matrix->width = strtol(nextChar, &nextChar, 10);//then the nextChar contains the result string from the fisrt strtol and it gets the next stringfrom the line

   matrix->Mtx = (float**)malloc((matrix->height) * sizeof(float*));//the matrix is assigned in the memory

    for(int i = 0; i < matrix->height; i++)
    {
        matrix->Mtx[i] = (float*)malloc((matrix->width) * sizeof(float));//it allocate all the rows by the number of columns
    }

    for(int i = 0; i < matrix->height; i++)//this for fill the matrix, it goes through all the rows by every columns
    {
      fgets(buffer, sizeof buffer, file);//it reads a line from the file
      nextChar = buffer;//the variable is equal to the buffer so that it can be used in the next loop
      for(int j = 0; j < matrix->width; j++)
      {
          (matrix->Mtx)[i][j] = strtof(nextChar, &nextChar);//with the strtof function we can read letter by letter and transform it into an float
      }
  }

  fclose(file);//the file is close
  return matrix;//the matrix read it is return
}

void WriteFile(Matrix* result)//the function recives the struct that result from the multiplication
{
  FILE* fp = fopen("result.txt", "w");//this variables is meant to write the file
  fprintf(fp, "%i %i\n", result->height, result->width);//the height and width are written in the file
  for(int i = 0; i < result->height; i++){//this two fors goes through all the rows by every columns
      for(int j = 0; j < result->width; j++){
          fprintf(fp, "%f ", (result->Mtx)[i][j]);//the data from each position of the matrix are written in the file
      }
      fprintf(fp, "\n");//"enter" to get the same format from the matrices files
  }
  printf("the resulting matrix was written in the file called result.txt \n");
  fclose(fp);//the file is close
}

void Multiply(Matrix* a, Matrix* b)
{

  Matrix * result;//a new struct is created to be used for the matrix result
  result = (Matrix*)malloc(sizeof (Matrix));//the struct is allocate in memory
  if((a->width) == (b->height))// if the number of columns from the first matrix are different from the rows of the second matrix the multiplication will be impossible to be done
  {
    /*the size from the final matrix will be the column
    from the first matrix and the row from the second
    and it will be equalize
    */
    result->height = a->height;
    result->width = b->width;

    result->Mtx = (float**)malloc((result->height) * sizeof(float*));//the matrix is assigned in the memory
    for(int i = 0; i < result->height; i++)
    {
        result->Mtx[i] = (float*)malloc((result->width) * sizeof(float));//it allocate all the rows by the number of columns
    }

    // Multiplying matrices a and b
    for(int i = 0; i < a->height; i++)//this for goes through all the rows in matrix a
    {
        for(int j = 0; j < b->width; j++)//this for goes through all the columns in matrix b
        {
            for(int k = 0; k < a->width; k++)//this for goes through all the columns in matrix a
            {
                (result->Mtx)[i][j] += (a->Mtx)[i][k] * (b->Mtx)[k][j];// storing result in result matrix

            }
        }
    }
    WriteFile(result);//this enters to the function WriteFile so it can be write in a txt file

    /*this liberate the structs allocated in memory */
    freeMemory(result);
    freeMemory(a);
    freeMemory(b);
  }
  else
  {
    printf("It can not multiply because the number of columns and rows are different \n");//display a error message
    /*this liberate the structs allocated in memory */
    freeMemory(result);
    freeMemory(a);
    freeMemory(b);
  }
}
