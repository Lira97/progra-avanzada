/*
	Enrique Lira Martinez
	A01023351
	Matrix multiplication
*/

#ifndef MATRIX_MULTIPLICATION_H
#define MATRIX_MULTIPLICATION_H

#include <stdio.h>
#include <stdlib.h>

typedef struct M //this is an struct for each matrix used
{
    int height;//this is the Height
    int width; //this is the Width
    float** Mtx;//this is the Matrix

} Matrix;

void freeMemory(Matrix* matrix);//function to free the memory, receives the structure to release
Matrix* readFile(char* NameFile);//function to read text file containing the data for the matrix, only receives the name of the file
void WriteFile(Matrix* result);//function that write the matrix resulting from the multiplicion in a text file, only receives the structure of the resulting matrix
void Multiply(Matrix* a, Matrix* b);//function that makes the multiplication of the matrix

#endif
