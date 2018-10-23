/*
	Enrique Lira Martinez
	A01023351
	Image Scaling
*/
#ifndef IMAGE_H
#define IMAGE_H

typedef struct pixel// struct to keep the RGB colors
{
    unsigned short R,G,B;// colors of the image
} pixel_t;

typedef struct Image// strcut to keep all the data from the image
{
    char magic_num[3];// varible that store the char, this indicate if the image is binary or ASCI
    int width;// varible that store size of the matrix
    int height;// varible that store size of the matrix
    int maxValue;// maximun colors
    pixel_t ** image;// matrix of pixels
} image_t;

void allocImage(image_t* image);// function to alloc the image in memory
void freeMemory(image_t* image);// function to free the image in memory

image_t* readImage(char* filename);// function to read the image in memory
void writeImage(char* filename, const image_t* image);// function to write the image in memory
void getBinaryImage(image_t* img, FILE* file);// function to get the values from the file
image_t* neagtiveColor(image_t* img);// function to invert the colors of the image
image_t* scaleImage(image_t* img, float factor);// function to scale  the image

#endif
