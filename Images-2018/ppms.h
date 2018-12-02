#ifndef PPMS_H
#define PPMS_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <math.h>
#include <omp.h>
#include <time.h>

// Assign constants to the colors
typedef enum color_enum {R, G, B} color_t;

// Structure to store a single pixel
typedef struct pixel_struct {
    // One byte: Allow max value to be 255
    unsigned char data[3];
    // Two bytes: Allow max value to be 65535
    //unsigned short data[3];
} pixel_t;

// Structure for the PPM image
typedef struct ppm_struct {
    char magic_number[3];
    int width;
    int height;
    int max_value;

    pixel_t ** pixels;
} ppm_t;

typedef struct matrix_struct {
    int width;
    int height;
    int ** values;
} Matrix;

///// FUNCTION DECLARATIONS

// Get the memory necessary to store an image of the size indicated in the structure
void allocateMemory(ppm_t * image);
void getValues(Matrix * matrix, FILE * file_ptr);
void readMatrixFile(char * filename, Matrix * matrix);
void allocateMemoryMatrix(Matrix * matrix);
void printValues(Matrix * image);
void Filter(ppm_t * image,Matrix * matrix);

void freeMemoryMatrix(Matrix * matrix);
// Release the memory for the image
void freeMemory(ppm_t * image);

// Open an image file and copy its contents to the structure
void readImageFile(char * filename, ppm_t * image);

// Write the data stored in a structure into an image file
void writeImageFile(char * filename, const ppm_t * image);

// Read each of the numerical values for the color components
//  and store them in the arrray for the pixels
void getAsciiPixels(ppm_t * image, FILE * file_ptr);

// Copy the bytes from the image into the data array for the pixels
void getBinaryPixels(ppm_t * image, FILE * file_ptr);

// Write the numerical values for the color components into
//  the output file, giving them a nice format
void writeAsciiPixels(const ppm_t * image, FILE * file_ptr);

// Copy the pixel data into the file as bytes
void writeBinaryPixels(const ppm_t * image, FILE * file_ptr);

// Compute the negative colors of an image
void getNegativeImage(ppm_t * image);

// Print the pixels of the image as hexadecimal on the console
void printPixels(ppm_t * image);

// Rotate the image a specified number of degrees
// Modify the structure received as a parameter
void rotateImage(ppm_t * source, int angle);

// Rotate the image and write the result in another structure
void rotateImage90(ppm_t * destination, const ppm_t * source);

// Rotate the image and write the result in another structure
void rotateImage180(ppm_t * destination, const ppm_t * source);

// Rotate the image and write the result in another structure
void rotateImage270(ppm_t * destination, const ppm_t * source);

// Scale an image by a given percentage
// Modify the structure received as a parameter
void scaleImage(ppm_t * source, int scale);

// Apply the scale to the pixels in an image
void resizeImage(ppm_t * destination, const ppm_t * source, int scale);

#endif  /* NOT PPMS_H */
