/*
    Enrique Lira Martinez A01023351
    Program for a simple bank server
    It uses sockets and threads
    The server will process simple transactions on a limited number of accounts

    Gilberto Echeverria
    gilecheverria@yahoo.com
    29/03/2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

// Custom libraries
#include "bank_codes.h"
#include "sockets.h"
#include "fatal_error.h"

#define NUM_ACCOUNTS 50
#define BUFFER_SIZE 15000
#define INITIAL_ALLOC 512
#define MAX_QUEUE 5

///// Structure definitions

// Data for a single bank account
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

///// FUNCTION DECLARATIONS
int interrupted = 0;
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

///// MAIN FUNCTION
int main(int argc, char * argv[]){
    int server_fd;
    bank_t bank_data;
    locks_t data_locks;

    printf("\n=== SIMPLE BANK SERVER ===\n");

    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }

    // Configure the handler to catch SIGINT
    setupHandlers();

    // Initialize the data structures
    initBank(&bank_data, &data_locks);

	// Show the IPs assigned to this computer
	printLocalIPs();
    // Start the server
    server_fd = initServer(argv[1], MAX_QUEUE);
	// Listen for connections from the clients
    waitForConnections(server_fd, &bank_data, &data_locks);
    // Close the socket
    close(server_fd);

    // Clean the memory used
    closeBank(&bank_data, &data_locks);

    // Finish the main thread
    pthread_exit(NULL);

    return 0;
}

///// FUNCTION DEFINITIONS

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {port_number}\n", program);
    exit(EXIT_FAILURE);
}

/*
    Modify the signal handlers for specific events
*/
void setupHandlers()
{
  struct sigaction new_action;

  // Configure the action to take
  // Block all signals during the time the handler funciton is running
  sigfillset(&new_action.sa_mask);
  new_action.sa_handler = detectInterruption;

  // Set the handler
  sigaction(SIGINT, &new_action, NULL);
}

// Signal handler
void detectInterruption(int signal)
{
    // Change the global variable
    printf("\nShutting down\n");
    interrupted = 1;

}
/*
    Function to initialize all the information necessary
    This will allocate memory for the accounts, and for the mutexes
*/
void initBank(bank_t * bank_data, locks_t * data_locks)
{
    // Set the number of transactions
    bank_data->total_transactions = 0;

    // Allocate the arrays in the structures
    bank_data->account_array = malloc(NUM_ACCOUNTS * sizeof (account_t));
    // Allocate the arrays for the mutexes
    data_locks->account_mutex = malloc(NUM_ACCOUNTS * sizeof (pthread_mutex_t));

    // Initialize the mutexes, using a different method for dynamically created ones
    //data_locks->transactions_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&data_locks->transactions_mutex, NULL);
    for (int i=0; i<NUM_ACCOUNTS; i++)
    {
        //data_locks->account_mutex[i] = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_init(&data_locks->account_mutex[i], NULL);
    }
}

/*
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd, bank_t * bank_data, locks_t * data_locks)
{
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    pthread_t new_tid;
    // Variables for polling
    int poll_response;
 	  int timeout = 500;		// Time in milliseconds

    // Get the size of the structure to store client information
    client_address_size = sizeof client_address;

    while (1)
    {
		//// POLL
        // Create a structure array to hold the file descriptors to poll
        struct pollfd test_fds[1];
        // Fill in the structure
        test_fds[0].fd = server_fd; // Polling stdin
        test_fds[0].events = POLLIN;    // Check for incomming data
        // Timeout finished without incidents
        poll_response = poll(test_fds, 1, timeout);

		// Error polling
        if (poll_response == -1)
        {
            // If poll detected an error due to an interruption, exit the loop
            if (errno == EINTR  && interrupted)
            {
                printf("THE PROGRAM WAS INTERRUPTED");
            }
            else
            {
                fatalError("ERROR: POLL");
            }
        }
		// Timeout finished without reading anything
        else if (poll_response == 0)
        {
            //printf("No response after %d seconds\n", timeout);
            if(interrupted)
            {
                printf("\nSERVER SHUTTING DOWN!\n");
                break;
            }
        }
		// the socket is ready
        else
        {
            // Server sent back a response
            if (test_fds[0].revents & POLLIN)
            {
				// Wait for client
				client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_size);
				if (client_fd == -1)
				{
					fatalError("ERROR: accept");
				}

				// Get the data from the client
				inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
				printf("Received incomming connection from %s on port %d\n", client_presentation, client_address.sin_port);

				// create a thead and prepare the structure to send to the thread
        thread_data_t* connection_data = malloc(sizeof(thread_data_t));
        connection_data->bank_data = bank_data;
        connection_data->data_locks = data_locks;
        connection_data->connection_fd = client_fd;
        int status;
        status = pthread_create(&new_tid, NULL, attentionThread, (void*) connection_data);
        if( status != 0)
        {
            printf("Failed to create handler!\n");
        }
        else{
            printf("Created thread %d for request.\n", (int)new_tid);
        }

            }
        }
    }
    // Print the number of transactions
    printf("Bank closed with: %i transactions.\n", getTransactions(bank_data, &(data_locks->transactions_mutex)));
}

void * attentionThread(void * arg){
  thread_data_t* data = (thread_data_t*) arg;
  printf("[%d] Attending request!\n", (int)pthread_self());

  // Stuff required by poll
  struct pollfd test_fds[1];
  int timeout = 10;
  int out = 0;
  int poll_result;
  char buffer[BUFFER_SIZE];
  int user = 0;
  long numbytes =0;
  inmsg_t message; //create strcut for message incomming
  while (!interrupted)
  {
      // POLL
      // Fill in the data for the structure
      test_fds[0].fd = data->connection_fd;
      test_fds[0].events = POLLIN;
      // Call poll
      poll_result = poll(test_fds, 1, timeout);

      if (poll_result == -1)
      {

          // If poll detected an error due to an interruption, exit the loop
          if(errno == EINTR && interrupted)
          {
              printf("KILLED BY FLAG\n");
              // An exit by a signal
              break;
          }
          else
          {
              fatalError("poll");
          }
      }
      else if (poll_result == 0){
          // Nothing happend

          // recive a  message

          while (out  != 1)
          {
            if(recvString(data->connection_fd, buffer, BUFFER_SIZE) == 0){
                printf("[%d] Client disconnected!\n", (int)pthread_self());//if the client disconnected
                break;
            }



            sscanf(buffer, "%i %s %i", (int*)&(message.op),(message.password),&(message.account));//revice the message from client
            printf("opcion :%u\n", message.op);
            if(message.op == EXIT)
            {// Client exit the program
                printf("[%d] Client is leaving!\n", (int)pthread_self());
                break;
            }
            if(interrupted)
            {
                break;
            }
            if(message.op == VERIFY)
            {
                printf("pass\n" );
              if (!checkValidAccount(message.account,message.password))
              {
              sprintf(buffer, "%i",  INCORRECT);
              sendString(data->connection_fd, buffer);
              }
              else
              {
                sprintf(buffer, "%i",  CORRECT);
                sendString(data->connection_fd, buffer);
                out = 1;
                printf("[%d] Successful transaction! - Total: %i\n", (int)pthread_self(), getTransactionsInThread(data));
                break;
              }

            }
            else
            {
              user = SaveUser(data, message.password);//Balance funtion
              sprintf(buffer, "%i %i",  REGISTERS,user);
              sendString(data->connection_fd, buffer);
            }

            printf("[%d] Successful transaction! - Total: %i\n", (int)pthread_self(), getTransactionsInThread(data));

          }

          if(recvString(data->connection_fd, buffer, BUFFER_SIZE) == 0){
              printf("[%d] Client disconnected!\n", (int)pthread_self());//if the client disconnected
              break;
          }

        //  inmsg_t message; //create strcut for message incomming
          message.optionalFileName = (char*)calloc(255, sizeof(char));
          message.optionalFilePath = (char*)calloc(255, sizeof(char));
          sscanf(buffer, "%i %f %ld %s %s", (int*)&(message.op),&(message.value),&numbytes, (message.optionalFileName), (message.optionalFilePath));//revice the message from client
          if(message.op == EXIT){// Client exit the program
              printf("[%d] Client is leaving!\n", (int)pthread_self());
              break;
          }

          printf("opcion :%u\n", message.op);
          unsigned char * file = (unsigned char*)calloc(numbytes, sizeof(unsigned char));
          int bytesTransfered = 0;
          int maxSize = 1024;
          int success = 0;
          if(maxSize > numbytes){
            maxSize = numbytes;
          }
          unsigned char * packet = (unsigned char*)calloc(BUFFER_SIZE, sizeof(unsigned char));
          switch(message.op){
              case RECEIVEFILE://receiveFile
              sprintf(buffer, "%i %d",  ASK, 10);//send an error
              sendString(data->connection_fd, buffer);
                int factor = 1;
                FILE *fp;
                fp = fopen("ServerFiles/glad.txt", "wb");
                while(numbytes > bytesTransfered){
                  read(data->connection_fd, packet, maxSize);
                  for(int i = 0; i < maxSize; i++){
                    if(bytesTransfered >= numbytes){
                      break;
                    }
                    file[bytesTransfered] = packet[i];
                    bytesTransfered++;
                  }
//                      int progress = (bytesTransfered*100)/numbytes;
                  printf("\r[%d] File Upload In progress %d bytes of %ld", (int)pthread_self(), bytesTransfered, numbytes);
                  write(data->connection_fd, "0", 1);
                  fflush(stdout);

                }
              free(packet);
              printf("\nFile uploaded\n");
              SaveFile(data, message.account,file,numbytes, message.optionalFileName, message.optionalFilePath, 0);
              free(file);
              break;
                  break;
              case DOWNLOADFILE://check the balance
                  break;
              case GETDIRECTORIES://Check Directories
                sprintf(buffer, "%i %d",  INSUFFICIENT, 10);//send an error
                sendString(data->connection_fd, buffer);
                struct dirent *de;  // Pointer for directory entry
                DIR *dr = opendir(".");
                if (dr == NULL){  // opendir returns NULL if couldn't open directory
                    printf("Could not open current directory" );
                    return 0;
                  }
                  char * dirs = (char*)calloc(5000, sizeof(char));
                  while ((de = readdir(dr)) != NULL) {
                      strcat(de->d_name, " ");
                      strcat(dirs, de->d_name);
                  }

                  sprintf(buffer, "%s", dirs);//send an error
                  sendString(data->connection_fd, buffer);
                  closedir(dr);
                break;
              case CHECKA://check the balance
                  sprintf(buffer, "%i",  ASK_ACCOUNT);//send an error
                  sendString(data->connection_fd, buffer);
                    if(recvString(data->connection_fd, buffer, BUFFER_SIZE) == 0){
                        printf("[%d] Client disconnected!\n", (int)pthread_self());//if the client disconnected
                        break;
                    }
                  sscanf(buffer, "%s",&(message.other_account));
                  FILE *fin;
                  char *line;
                  char file[100];
                  char account[100];
                  sprintf(account, "%d", message.account);
                  strcpy(file, &message.other_account);
                  strcat(file, "/account.txt");
                  printf("%s\n",account );
                  printf("%s\n", &message.other_account);
                  printf("%s\n", file);
                  fin = fopen(file, "rb");
                  if ( fin )
                  {
                      while (( line = read_line_by_line(fin)))
                      {
                          if ( strstr(line,account) )
                          {
                            printf("%s\n", line );
                            sprintf(buffer, "%i",  FOUND);//send an error
                            sendString(data->connection_fd, buffer);
                            break;
                          }
                          free(line);
                      }
                  }
                  sprintf(buffer, "%i",  NOFOUND);//send an error
                  sendString(data->connection_fd, buffer);
                  fclose(fin);
                    break;
                  case GIVE://check the balance
                        sprintf(buffer, "%i",  ASK_ACCOUNT);//send an error
                        sendString(data->connection_fd, buffer);
                          if(recvString(data->connection_fd, buffer, BUFFER_SIZE) == 0){
                              printf("[%d] Client disconnected!\n", (int)pthread_self());//if the client disconnected
                              break;
                          }
                        sscanf(buffer, "%s",&(message.other_account));
                        AddPersmisses(data ,message.account,&message.other_account);
                        sprintf(buffer, "%i",  WRITTEN);//send an error
                        sendString(data->connection_fd, buffer);
                        fclose(fin);
                          break;
              case CREATEDIRECTORY://check the balance
                Delete(data, message.account);
                sprintf(buffer, "%i",  OK);
                sendString(data->connection_fd, buffer);
                break;
              case DELETE://check the balance
                Delete(data, message.account);
                sprintf(buffer, "%i",  OK);
                sendString(data->connection_fd, buffer);
                break;
              default:
                  fatalError("Unknown code!");//type a wrong option
          }
          printf("[%d] Successful transaction! - Total: %i\n", (int)pthread_self(), getTransactionsInThread(data));
      }
  }
  sprintf(buffer, "%i %d",  BYE, 0);//finish the connection
  sendString(data->connection_fd, buffer);
  free(data);
  pthread_exit(NULL);
}


/*
    Free all the memory used for the bank data
*/
void closeBank(bank_t * bank_data, locks_t * data_locks)
{
    printf("DEBUG: Clearing the memory for the thread\n");
    free(bank_data->account_array);
    free(data_locks->account_mutex);
}

/*
    Return true if the account provided is within the valid range,
    return false otherwise
*/
char * Encrypt_password(char s[])	// OR char return_and_pass(char *s)
{
 	char ch;
  int i;
  int key = 4;
      for(i = 0; s[i] != '\0'; ++i)
      {
          ch = s[i];

          if(ch >= 'a' && ch <= 'z'){
              ch = ch + key;

              if(ch > 'z'){
                  ch = ch - 'z' + 'a' - 1;
              }

              s[i] = ch;
          }
          else if(ch >= 'A' && ch <= 'Z'){
              ch = ch + key;

              if(ch > 'Z'){
                  ch = ch - 'Z' + 'A' - 1;
              }

              s[i] = ch;
          }
      }
      return(s);
}
int checkValidAccount(int account,char password[])
{
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  char result[255];
  char result2[255];
  FILE *fp;
  int count=0;
  fp = fopen("passwords.txt", "r");
  if (fp == NULL)
   return 0;

   while ((read = getline(&line, &len, fp)) != -1)
  {
   if (count == account)
   {
       if (line[read-1] == '\n')
           line[read-1] = 0;
           strcpy(result2, line);
      }
        count++;
    }
  fclose(fp);
  strcpy(result, Encrypt_password(password));
  return (account >= 0 && account < NUM_ACCOUNTS && strcmp(result,result2)==0);
}
int SaveUser(thread_data_t* data ,char password[])
{

    pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);
    int account =0;

    pthread_mutex_lock(transaction);

      char * line = NULL;
      size_t len = 0;
      ssize_t read;
      FILE *fp;

      fp = fopen("passwords.txt", "rb+");
      if (fp == NULL)
       return 0;

       while ((read = getline(&line, &len, fp)) != -1)
      {
        account++;
      }
    fprintf(fp,"%s\n",Encrypt_password(password));
    free(line);
    fclose(fp);

    pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second

    return account;
}
int AddPersmisses(thread_data_t* data ,int account_num,char other_account[])
{
    account_t* account = &(data->bank_data->account_array[account_num]);
    pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
    pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);
    size_t len = 0;
    ssize_t read;
    char * line = NULL;
    char file[100];

    pthread_mutex_lock(transaction);

    sprintf(file, "%d", account_num);
    strcat(file, "/account.txt");
    printf("%s\n",other_account );
    printf("%s\n", file);


      account->infile = fopen(file, "rb+");
      if(account->infile == NULL)
        return 0 ;
        while ((read = getline(&line, &len, account->infile)) != -1)
       {
         account++;
       }

       fprintf(account->infile,"%s\n",other_account);
    free(line);
    fclose(account->infile);
    pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second
    pthread_mutex_unlock(account_l);
    return 0;
}
void SaveFile(thread_data_t* data, int account_num,unsigned char * message,int numbytes, char * fileName, char * path, int flag){
    account_t* account = &(data->bank_data->account_array[account_num]);
    pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
    pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);

    pthread_mutex_lock(account_l);// Locks account first and transactions second
    pthread_mutex_lock(transaction);
    strcat(path, fileName);
    account->infile = fopen(path, "wb+");
    if(account->infile == NULL)
      return ;

    if(message == NULL)
      return ;

    /* copy all the text into the buffer */
    fwrite (message , sizeof(char),numbytes, account->infile);
    fclose(account->infile);

    /* confirm we have read the file by
    outputing it to the console */
    pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second
    pthread_mutex_unlock(account_l);

}
void Delete(thread_data_t* data, int account_num)
{
    account_t* account = &(data->bank_data->account_array[account_num]);
    pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
    pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);

    pthread_mutex_lock(account_l);// Locks account first and transactions second
    pthread_mutex_lock(transaction);
    int status;

    printf("Enter name of a file you wish to delete\n");

    status = remove("ServerFiles/output.mp4");

    if (status == 0)
      printf("file deleted successfully.\n");
    else
    {
      printf("Unable to delete the file\n");
      perror("Following error occurred");
    }

    pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second
    pthread_mutex_unlock(account_l);


}
int getTransactionsInThread(thread_data_t* data)
{
    return getTransactions(data->bank_data, &(data->data_locks->transactions_mutex));
}
/**
 * Gets Transactions
 */
int getTransactions(bank_t* bank_data, pthread_mutex_t* transaction)
{
    int value;
    pthread_mutex_lock(transaction);
    value = bank_data->total_transactions;
    pthread_mutex_unlock(transaction);
    return value;
}

char * read_line_by_line(FILE *fin)
{
    char *buffer;
    char *tmp;
    int read_chars = 0;
    int bufsize = INITIAL_ALLOC;
    char *line = malloc(bufsize);

    if ( !line ) {
        return NULL;
    }

    buffer = line;

    while ( fgets(buffer, bufsize - read_chars, fin) )
    {
      printf("%s\n",line );
        read_chars = strlen(line);

        if ( line[read_chars - 1] == '\n' )
        {
          line[read_chars - 1] = '\0';
          return line;
        }

        else {
            bufsize = 2 * bufsize;
            tmp = realloc(line, bufsize);
            if ( tmp ) {
                line = tmp;
                buffer = line + read_chars;
            }
            else {
                free(line);
                return NULL;
            }
        }
    }
    return NULL;
}
