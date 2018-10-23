/*
	Enrique Lira Martinez
	A01023351
  Fence rail cipher
*/
#ifndef VIGENERE_WORKER_H
#define VIGENERE_WORKER_H

#define BUFFER_SIZE 512//buffer for the pipe

char* name_file(char* filename, char action);//funtio for the name of the file
void readPipe(int file_descriptor[]);//function for the reding pipe
void child(char action, int key, char* fileName, FILE* src_file, int* fd);//fucntion child for the reading

#endif
