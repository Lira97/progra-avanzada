/*
	Enrique Lira Martinez
	A01023351
	Image Scaling
  How to use:

      To run this program
      you should compile like this : "gcc image.c main.c -o main" or you can run the makefile
      and exceute like this: "./main -i ninja-icon.ppm -o scale.ppm -n -s 200"

      then the program will create a file with the scaleted image and other with the negative colors called negative.ppm
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "image.h"

void allocImage(image_t* image)
{


    image->image = (pixel_t**)malloc((image->height) * sizeof(pixel_t*));//the matrix is assigned in the memory with the pixel struct in each position
    image->image = (pixel_t**)calloc( image->width, sizeof( pixel_t*) );// Dynamically allocate 2D array and initialize its row and count
    for(int i = 0; i < (image->height); i++)//passthrough all the elements
    {
         image->image[i] = (pixel_t*)calloc( image->height, sizeof( pixel_t* ) );//equalize the value to 0
    }
}
void freeMemory(image_t* image)
{

    free(image->image);// the image in the struct is delete
    free(image);//as well as the entire struct
}

image_t* readImage(char* filename)
{
    FILE* file = fopen(filename, "rb");//open a file that contains binary data
    image_t * img = malloc(sizeof(image_t));//the struct is allocate in memory

    fgets(img->magic_num, 3, file);//it gets the number
    img->magic_num[2] = '\0';
    fscanf(file, "%i %i", &(img->height), &(img->width));//it gets the size of the picture
    fscanf(file, "%i\n", &(img->maxValue));//this is the maximun range of color
    allocImage(img);//fucntion that allocate the entire image

    if(strcmp("P3", img->magic_num) == 0)//it only reads binary images
    {
        printf("This is not a binary image");// error message
        exit(1);
    }
    else if(strcmp("P6", img->magic_num) == 0)
    {
        getBinaryImage(img, file);//enter the function that reads the image

    }
    else
    {
      printf("This is not a binary image");// error message
      exit(1);
    }
    fclose(file);//the file is close
    return img;//the image read it is return
}
void getBinaryImage(image_t* img, FILE* file)
{
    for(int i = 0; i < img->height; i++)//this loop pass to all the elements in the matrix image
    {
        for(int j = 0; j < img->width; j++)
        {
          fread(&img->image[i][j].R, 1, 1, file);// get the color red from the file and put it in the struct pixel
          fread(&img->image[i][j].G, 1, 1, file);// get the color green from the file and put it in the struct pixel
          fread(&img->image[i][j].B, 1, 1, file);// get the color blue from the file and put it in the struct pixel
        }
    }
}
void writeImage(char* filename, const image_t* img)
{
    FILE* file = fopen(filename, "w");// it opens the file
    fprintf(file, "P6\n");// start writting the magic number for binary file
    fprintf(file, "%i %i\n", img->height, img->width);//it write the size of the new image
    fprintf(file, "%i\n", img->maxValue);// write the max value for the colors

    for(int i = 0; i < img->height; i++)//this loop pass to all the elements in the matrix image
    {
        for(int j = 0; j < img->width; j++)
        {
          fwrite(&img->image[i][j].R, 1, 1, file);// write the color red from the struct pixel and put it in the file
          fwrite(&img->image[i][j].G, 1, 1, file);// write the color green from the struct pixel and put it in the file
          fwrite(&img->image[i][j].B, 1, 1, file);// write the color blue from the struct pixel and put it in the file
        }
    }
    fclose(file);//the file is close
}
image_t* neagtiveColor(image_t* img)
{
  image_t * n_img ;// create a new image
  n_img = (image_t*)malloc(sizeof(image_t));// the struct is allocate in memory
  n_img -> height = img ->height;// equalize the height of the original image
  n_img -> width = img ->width;// equalize the width of the original image
  strncpy(n_img->magic_num, img->magic_num, 3);// then copy the magic number that is a string
  n_img -> maxValue = img ->maxValue;// equalize the max value of the original image
  allocImage(n_img);// fucntion that allocate the entire image
    for(int i = 0; i < img->height; i++)// this loop pass to all the elements in the matrix image
    {
        for(int j = 0; j < img->width; j++)
        {
            n_img->image[i][j].R = img->maxValue - img->image[i][j].R;// subtract the maximum value from the color red
            n_img->image[i][j].G = img->maxValue - img->image[i][j].G;// subtract the maximum value from the color green
            n_img->image[i][j].B = img->maxValue - img->image[i][j].B;// subtract the maximum value from the color blue
        }
    }
    return n_img;//the image read it is return
}

image_t* scaleImage(image_t* img, float factor)
{
  image_t * n_img ;// create a new image

  n_img = (image_t*)malloc(sizeof(image_t));// the struct is allocate in memory
  n_img -> height = img ->height*(factor/100);// equalize the height divide with the factor of the original image
  n_img -> width = img ->width*(factor/100);// equalize the width divide with the factor of the original image
  strncpy(n_img->magic_num, img->magic_num, 3);// then copy the magic number that is a string
  n_img -> maxValue = img ->maxValue;// equalize the max value of the original image
  allocImage(n_img);// fucntion that allocate the entire image

  if((factor/100)>=1)//if the factor is higher than the 100% of the image ,it will do the function below
  {
    for (int i=0;i<n_img -> height;i++)// this loop pass to all the elements of the new matrix image
    {
      for (int j=0;j<n_img -> width;j++)
      {
        // it looks for the nearest pixel and places it in the position
        n_img-> image[i][j] = img -> image[(int)(floor(i/(factor/100)))][(int)(floor(j/(factor/100)))];
      }
    }
  }
  else if ((factor/100)== 0.5)//if the factor is equal to 50%
  {
    int x=0;
    int y=0;
    for(int i = 0; i < n_img->height; i++)// this loop pass to all the elements of the new matrix image
    {
      for(int j = 0; j < n_img->width; j++)
      {
        // This takes the average of the pixel moves from 4 to 4 pixels
        n_img->image[i][j].R = ((img->image[x][y].R+img->image[x+1][y].R+img->image[x][y+1].R+img->image[x+1][y+1].R)/4);
        n_img->image[i][j].G = ((img->image[x][y].G+img->image[x+1][y].G+img->image[x][y+1].G+img->image[x+1][y+1].G)/4);
        n_img->image[i][j].B = ((img->image[x][y].B+img->image[x+1][y].B+img->image[x][y+1].B+img->image[x+1][y+1].B)/4);
        y=j*2;
      }
      x=i*2;
    }
  }
  else
  {
    printf("The only downscaling that this program can do is 50 percent \n");// error message
    exit(1);
  }
  return n_img;
}
