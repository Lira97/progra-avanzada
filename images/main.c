/*
	Enrique Lira Martinez
	A01023351
	Image Scaling
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "image.h"

int main(int argc, char** argv)
{
  int option = 0;
  char *inputFile = NULL;
  char *outputFile = NULL;
  int factor = 0;
  int negative = 0;

  while((option = getopt(argc, argv, "s:i:o:n")) != -1 )//the getopt is used to read the user's input and output
  {
    switch(option)//The switch is used to verify the input or output
    {
      case 'i':

        inputFile = optarg;//the argument -i is read and converted into an integer to be used in the following functions
        break;
      case 'o':
        outputFile = optarg;//the argument -o is read and converted into an integer to be used in the following functions
        break;
      case 'n':
        negative=1;//the argument -o is read and converted into an integer to be used in the following functions
        break;
      case 's':
        factor = atoi(optarg);//the argument -o is read and converted into an integer to be used in the following functions
        break;
      default:
        break;
    }
  }
  image_t* img = readImage(inputFile);// it read the image from the arguments

  if(negative == 1)// if the user choose to get the negative it will enter to this condition
  {
    image_t* img2 = neagtiveColor(img);// enter to the function to convert in negative
    writeImage("negative.ppm", img2);// and write the image in a new file
    printf("The image is saved in the file negative.ppm\n");
    freeMemory(img2);// the memory get free
  }
  image_t* img3 = scaleImage(img,factor);// enter to the function to convert in scale
  writeImage(outputFile, img3);// and write the image in a new file 
  printf("The image is saved in the file %s\n", outputFile);
  freeMemory(img);// the memory get free
  freeMemory(img3);// the memory get free

  return 0;
}
