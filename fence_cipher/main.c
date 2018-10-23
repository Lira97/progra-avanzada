/*
	Enrique Lira Martinez
	A01023351
  Fence rail cipher
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "cipher.h"
#include "readfile.h"

int main()
{
  while(1)
  {
    char option;
    printf("\nSelect an option:\nEncode or Decode? (E/D)\nQuit (Q)\n");//ask to the user the option
    option = getchar();//get the imput from the keyboard
    switch(option)//start the menu for the user
    {
      case 'D':
      option = 0; // if the user chose decoding
      break;
      case 'E':
      option = 1; // if the user chose encoding
      break;
      case 'Q'://close the progrma
      printf("ENDING PROGRAM!\n");
      return 0;
      default:
      printf("INVALID\n");//if the user fail
      continue;
    }

    printf("Enter the file name:\n");//ask the file
    char filename[50];
    scanf("%s", filename);//recive the name
    FILE* file = fopen(filename, "r");//open the file


    if(file == NULL)
    {
      printf("File not found!\n");//if the file doesn't exist it will continue
      fflush(stdout);//flush the input
      fflush(stdin);//flush the input
      continue;
    }

    printf("Please provide a key:\n");//ask the key for the cipher method
    int key=0;
    scanf("%d", &key);//recive the key


    int fd_c2f[2];//create the pipe
    if(pipe(fd_c2f)== -1)//if the pipe gets an error
    {
      printf("Error!\nQuitting!\n");// Error creating the pipe
      exit(EXIT_FAILURE);
    }


    pid_t pid;
    pid = fork();// fork

    if(pid == 0)
    {
      child(option, key, filename, file, fd_c2f);// Call the child function
      exit(0);
    }
    else if (pid > 0)
    {
      readPipe(fd_c2f);// Read from the function pipe
      fflush(stdin);//flush the input
    }
    else
    {
      printf("Error!\nQuitting!\n");// Error creating the pipe
      exit(EXIT_FAILURE);
    }
  }
  return 0;
}
