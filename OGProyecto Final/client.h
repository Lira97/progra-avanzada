#ifndef CLIENT_H
#define CLIENT_H
#define BUFFER_SIZE 1024
#include <stdbool.h>


//Struct to keep user's info.
typedef struct userInfo {
	int account;
	char * password;
	float value;
	char * homeDirectory;
	char * optionalFileName;
	char * optionalFilePath;
} userStruct;



///// FUNCTION DECLARATIONS
void usage(char * program);
void bankOperations(int connection_fd);
bool downloadFile(float numbytes, float bytesTransfered, float bytesReceived, unsigned char * packet, int connection_fd, char * fileName);
bool uploadFile(float numbytes, float bytesTransfered, unsigned char * packet, int connection_fd, char * filePath, char * filename);
#endif
