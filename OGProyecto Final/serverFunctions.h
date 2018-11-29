/*
	Enrique Lira Martinez A01023351 ,Emiliano Abascal Gurria A01023234, Cesar Armando Valladares A01023503
	Client program to access nas
	This program connects to the server using sockets

	Gilberto Echeverria
	gilecheverria@yahoo.com
	29/03/2018
*/


#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
// Signals library
#include <errno.h>
#include <signal.h>
// Sockets libraries
#include <netdb.h>
#include <sys/poll.h>
// Posix threads library
#include <pthread.h>
#include <dirent.h>

#include "codes.h"


#define NUM_ACCOUNTS 50
#define BUFFER_SIZE 1024
#define INITIAL_ALLOC 512
#define MAX_QUEUE 5


typedef struct account_struct {
	int id;
	float balance;
	FILE *infile;
} account_t;

// Data for the nas operations
typedef struct nas_struct {
	// Store the total number of operations performed
	int total_transactions;
	// An array of the accounts
	account_t * account_array;
} nas_t;

// Structure for the mutexes to keep the data consistent
typedef struct locks_struct {
	// Mutex for the number of transactions variable
	pthread_mutex_t transactions_mutex;
	// Mutex array for the operations on the accounts
	pthread_mutex_t * account_mutex;
} locks_t;

// Data that will be sent to each structure
typedef struct data_struct {
	// The file descriptor for the socket
	int connection_fd;
	// A pointer to a nas data structure
	nas_t * nas_data;
	// A pointer to a locks structure
	locks_t * data_locks;
} thread_data_t;

// Used to contain incomming messages
typedef struct incomming_struct {
	int account;
	operation_t op;
	char password[255];
	float value;
	char other_account;
	char * userHomeDirectory;
	char * optionalFileName;
	char * optionalFilePath;
} inmsg_t;


void usage(char * program);
void setupHandlers();
void initNAS(nas_t * nas_data, locks_t * data_locks);
void waitForConnections(int server_fd, nas_t * nas_data, locks_t * data_locks);
void * attentionThread(void * arg);
int checkValidAccount(thread_data_t* data ,int account_num,char password[]);
int checkPermissions(thread_data_t* data ,int account_num,char *other_account);
void closenas(nas_t * nas_data, locks_t * data_locks);
void detectInterruption(int signal);
void SaveFile(thread_data_t* data, int account_num,unsigned char * message,int numbytes, char * fileName, char * path);
int Delete(thread_data_t* data,int account_num,char *fileName,char * path);
int DeletePermissions(thread_data_t* data ,int account_num,char other_account[]);
// Custom functions
char * read_line_by_line(FILE *fin) ;
int AddPersmisses(thread_data_t* data ,int account_num,char *other_account,char * path);
int SaveUser(thread_data_t* data ,char password[]);
int getTransactionsInThread(thread_data_t* data);
int getTransactions(nas_t* nas_data, pthread_mutex_t* transaction);
long readFile(thread_data_t* data, int account_num, char * fileName, char * path);
bool serverDownloadFile(float numbytes, float bytesTransfered, float bytesReceived, unsigned char * packet, int connection_fd, char * fileName);
bool serverUploadFile(float numbytes, float bytesTransfered, unsigned char * packet, int connection_fd, char * filePath);


#endif
