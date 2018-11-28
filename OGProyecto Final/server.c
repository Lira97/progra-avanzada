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

#include "sockets.h"
#include "serverFunctions.h"
#include "fatal_error.h"

///// Structure definitions

// Data for a single bank account


///// FUNCTION DECLARATIONS
int interrupted = 0;


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
  int fileMaxSize = 1024;
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

          if(fileMaxSize > numbytes){
            fileMaxSize = numbytes;
          }
          unsigned char * packet = (unsigned char*)calloc(BUFFER_SIZE, sizeof(unsigned char));
          switch(message.op){
              case RECEIVEFILE://receiveFile
              sprintf(buffer, "%i %d",  ASK, 10);//send an error
              sendString(data->connection_fd, buffer);
                while(numbytes > bytesTransfered){
                  read(data->connection_fd, packet, fileMaxSize);
                  for(int i = 0; i < fileMaxSize; i++){
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
                  if (fin){
                      while (( line = read_line_by_line(fin))){
                          if ( strstr(line,account) ){
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
                        recvString(data->connection_fd, buffer, BUFFER_SIZE);
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
