/*
	Enrique Lira Martinez
	A01023351
  Fence rail cipher
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cipher.h"
#include "readfile.h"
#define PIPE_BUFFER_SIZE 512

char* name_file(char* filename, char option)
{
    int length = strlen(filename);//get the size for the reusulting name

    char* extension = malloc((length + 8 + 1) * sizeof(char));//create a char for the name

    if(option)
    {
        strcpy(extension, "encoded_");//copy the string for the name
    }
    else
    {
        strcpy(extension, "decoded_");//copy the string for the name
    }
    strcat(extension, filename);//at the end it concatenate the string with the acticon chose

    return extension;
}
void readPipe(int file_descriptor[])
{
    FILE * file = NULL;
    char buffer[PIPE_BUFFER_SIZE];

    close(file_descriptor[1]);// Close the descriptor for writing

    file = fdopen(file_descriptor[0], "r"); // Open file pointer

    while ( fgets(buffer, PIPE_BUFFER_SIZE, file) )// Get the data from the stream
    {
        printf("\nThe result was written in: %s\n", buffer);
    }
    fclose(file);  // Close the file pointer

    close(file_descriptor[0]);// Close the file descriptor
}
void child(char option, int key, char* fileName, FILE* file, int* fd)
{
    close(fd[0]);// Close file descriptor

    FILE* pipe = fdopen(fd[1], "w");// Open file with the file descriptor

    char* outfileName = name_file(fileName, option);// Generate the name of the new file

    FILE* out = fopen(outfileName, "w");// Open the writting file

    char buffer[BUFFER_SIZE];//char for the line read it

    while(fgets(buffer, BUFFER_SIZE, file))// Start reading from the file
    {
      if(buffer[strlen(buffer)-1] == '\n')
      {
        buffer[strlen(buffer)-1] = '\0';
      }
      if(option)//choose between the two opcions
      {
        fprintf(out, "%s\n",encode(buffer,key));// write the encoding fuction
      }
      else
      {
        fprintf(out, "%s\n",decrypt(buffer,key));// write the decoding fuction
      }
    }
    // Close files
    fclose(file);
    fclose(out);

    fprintf(pipe, "%s", outfileName);  // Report back to main

    free(outfileName);  // Free allocated strings

    fclose(pipe);// Close pipe
    close(fd[1]);
}
