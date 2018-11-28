#ifndef SERVER_H
#define SERVER_H


#define NUM_ACCOUNTS 50
#define BUFFER_SIZE 15000
#define INITIAL_ALLOC 512
#define MAX_QUEUE 5


typedef struct account_struct {
	int id;
	float balance;
	FILE *infile;
} account_t;

// Data for the bank operations
typedef struct bank_struct {
	// Store the total number of operations performed
	int total_transactions;
	// An array of the accounts
	account_t * account_array;
} bank_t;

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
	// A pointer to a bank data structure
	bank_t * bank_data;
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
	char * optionalFileName;
	char * optionalFilePath;
} inmsg_t;


void usage(char * program);
void setupHandlers();
void initBank(bank_t * bank_data, locks_t * data_locks);
void waitForConnections(int server_fd, bank_t * bank_data, locks_t * data_locks);
void * attentionThread(void * arg);
int checkValidAccount(int account,char password[]);
void closeBank(bank_t * bank_data, locks_t * data_locks);
void detectInterruption(int signal);
void SaveFile(thread_data_t* data, int account_num,unsigned char * message,int numbytes, char * fileName, char * path, int flag);
void Delete(thread_data_t* data, int account_num);
// Custom functions
float Deposit(thread_data_t* data, int account_num, float amount);
char * read_line_by_line(FILE *fin) ;
int AddPersmisses(thread_data_t* data ,int account_num,char other_account[]);
int SaveUser(thread_data_t* data ,char password[]);
int getTransactionsInThread(thread_data_t* data);
int getTransactions(bank_t* bank_data, pthread_mutex_t* transaction);


#endif
