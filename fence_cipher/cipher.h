/*
	Enrique Lira Martinez
	A01023351
  Fence rail cipher
*/
#ifndef CIPHER_H
#define CIPHER_H

typedef struct matrix// strcut to keep all the data from the image
{
    int width;// varible that store size of the matrix
    int height;// varible that store size of the matrix
    char ** railMatrix;// matrix of pixels
} matrix_t;

void allocImage(matrix_t* matrix);// function to alloc the image in memory
void freeMemory(matrix_t* matrix);// function to free the image in memory

char * encode(char * text, int key);// function to encode the message
char * decrypt(char * text, int key);// function to decode the message
#endif
