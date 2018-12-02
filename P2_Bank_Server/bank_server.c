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

// Custom libraries
#include "bank_codes.h"
#include "sockets.h"
#include "fatal_error.h"

#define NUM_ACCOUNTS 4
#define BUFFER_SIZE 1024
#define MAX_QUEUE 5

///// Structure definitions

// Data for a single bank account
typedef struct account_struct {
    int id;
    float balance;
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
    float value;
} inmsg_t;

///// FUNCTION DECLARATIONS
int interrupted = 0;
void usage(char * program);
void setupHandlers();
void initBank(bank_t * bank_data, locks_t * data_locks);
void waitForConnections(int server_fd, bank_t * bank_data, locks_t * data_locks);
void * attentionThread(void * arg);
int checkValidAccount(int account);
void closeBank(bank_t * bank_data, locks_t * data_locks);
void detectInterruption(int signal);

// Custom functions
float Deposit(thread_data_t* data, int account_num, float amount);
float Balance(thread_data_t* data, int account_num);
int getTransactionsInThread(thread_data_t* data);
int getTransactions(bank_t* bank_data, pthread_mutex_t* transaction);
float Withraw(thread_data_t* data, int account_num, float amount);
/*
    EXAM: Add your function declarations here
*/



///// MAIN FUNCTION
int main(int argc, char * argv[])
{
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

void * attentionThread(void * arg)
{
  thread_data_t* data = (thread_data_t*) arg;
  printf("[%d] Attending request!\n", (int)pthread_self());

  // Stuff required by poll
  struct pollfd test_fds[1];
  int timeout = 10;
  int poll_result;
  char buffer[BUFFER_SIZE];
  float balance = 0;
  float withraw = 0;
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
      else if (poll_result == 0)
      {
          // Nothing happend
      }
      else
      {
          // recive a  message
          if(recvString(data->connection_fd, buffer, BUFFER_SIZE) == 0)
          {
              printf("[%d] Client disconnected!\n", (int)pthread_self());//if the client disconnected
              break;
          }

          inmsg_t message; //create strcut for message incomming

          sscanf(buffer, "%i %i %f", (int*)&(message.op),&(message.account),&(message.value));//revice the message from client

          if(message.op == EXIT)// Client exit the program
          {
              printf("[%d] Client is leaving!\n", (int)pthread_self());
              break;
          }
          if(!checkValidAccount(message.account))   // Make sure account is valid
          {
              sprintf(buffer, "%i %d",  NO_ACCOUNT, 0);
              sendString(data->connection_fd, buffer);
              continue;
          }

          if(interrupted)
          {
              break;
          }

          switch(message.op)
          {
              case CHECK://check the balance
                  balance = Balance(data, message.account);//Balance funtion
                  break;
              case DEPOSIT:// give money to the account
                  if(message.value < 0)// it can't can be negative number
                  {
                      sprintf(buffer, "%i %d",  ERROR, 0);//send an error
                      sendString(data->connection_fd, buffer);
                      continue;
                  }
                  // Make deposit
                  balance = Deposit(data, message.account, message.value);
                  break;
              case WITHDRAW:
                  if(message.value < 0)// it can't can be negative number
                  {
                    sprintf(buffer, "%i %d",  ERROR, 0);//send an error
                    sendString(data->connection_fd, buffer);
                    continue;
                  }
                  withraw = Withraw(data, message.account, message.value);//Withraw funtion
                  if(withraw >= 0)//the number must be 0 or higher
                  {
                    balance = withraw;//get the balance
                    data->bank_data->total_transactions++;//add a transaction
                    break;
                  }
                  else
                  {
                    sprintf(buffer, "%i %d",  INSUFFICIENT, 0);//send an error
                    sendString(data->connection_fd, buffer);
                    continue;
                  }
                  break;
              default:
                  fatalError("Unknown code!");//type a wrong option
          }

          // If there is no error it send and ok
          sprintf(buffer, "%i %f",  OK, balance);
          sendString(data->connection_fd, buffer);
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
int checkValidAccount(int account)
{
    return (account >= 0 && account < NUM_ACCOUNTS);
}
float Balance(thread_data_t* data, int account_num)
{
    account_t* account = &(data->bank_data->account_array[account_num]);
    pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
    pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);
    float value;

    pthread_mutex_lock(account_l);// Locks account first and transactions second
    pthread_mutex_lock(transaction);

    value = account->balance;
    printf("%f\n", value);
    data->bank_data->total_transactions++;

    pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second
    pthread_mutex_unlock(account_l);

    return value;
}

float Deposit(thread_data_t* data, int account_num, float amount)
{
    account_t* account = &(data->bank_data->account_array[account_num]);
    pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
    pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);
    float value;


    pthread_mutex_lock(account_l);// Locks account first and transactions second
    pthread_mutex_lock(transaction);

    account->balance += amount;
    value = account->balance;
    data->bank_data->total_transactions++;

    pthread_mutex_unlock(transaction);// Unlocks transactions first and account second
    pthread_mutex_unlock(account_l);

    return value;
}
float Withraw(thread_data_t* data, int account_num, float amount)
{
    account_t* account = &(data->bank_data->account_array[account_num]);
    pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
    pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);
    float value;

    pthread_mutex_lock(account_l);// Locks account first and transactions second
    pthread_mutex_lock(transaction);

    account->balance -= amount;
    value = account->balance;

    pthread_mutex_unlock(transaction);// Unlocks transactions first and account second
    pthread_mutex_unlock(account_l);
    return value;
}
/**
 * Wrapper to get transactions from within the threads
 */
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
