/*
  Enrique Lira Martinez A01023351 ,Emiliano Abascal Gurria A01023234, Cesar Armando Valladares A01023503
  Client program to access nas
  This program connects to the server using sockets

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

int interrupted = 0;

int main(int argc, char * argv[]){
    int server_fd;
    nas_t nas_data;
    locks_t data_locks;

    printf("\n=== SIMPLE NAS SERVER ===\n");

    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }

    // Configure the handler to catch SIGINT
    setupHandlers();

    // Initialize the data structures
    initNAS(&nas_data, &data_locks);

	// Show the IPs assigned to this computer
	printLocalIPs();
    // Start the server
    server_fd = initServer(argv[1], MAX_QUEUE);
	// Listen for connections from the clients
    waitForConnections(server_fd, &nas_data, &data_locks);
    // Close the socket
    close(server_fd);

    // Clean the memory used
    closenas(&nas_data, &data_locks);

    // Finish the main thread
    pthread_exit(NULL);

    return 0;
}
//Explanation to the user of the parameters required to run the program
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {port_number}\n", program);
    exit(EXIT_FAILURE);
}
//Modify the signal handlers for specific events
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
void initNAS(nas_t * nas_data, locks_t * data_locks)
{
    // Set the number of transactions
    nas_data->total_transactions = 0;

    // Allocate the arrays in the structures
    nas_data->account_array = malloc(NUM_ACCOUNTS * sizeof (account_t));
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
void waitForConnections(int server_fd, nas_t * nas_data, locks_t * data_locks)
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
        connection_data->nas_data = nas_data;
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
    printf("nas closed with: %i transactions.\n", getTransactions(nas_data, &(data_locks->transactions_mutex)));
}

void * attentionThread(void * arg){
  thread_data_t* data = (thread_data_t*) arg;
  printf("[%d] Attending request!\n", (int)pthread_self());
  // Stuff required by poll
  //Variable declaration.
  unsigned char * packet = NULL;
  int fileMaxSize = 1024;
  int timeout = 10;
  int out = 0;
  int poll_result;
  int user = 0;
  long numbytes =0;
  char buffer[BUFFER_SIZE];
  inmsg_t message; //create strcut for message incomming
  struct pollfd test_fds[1];
  message.optionalFileName = (char*)calloc(255, sizeof(char));
  message.optionalFilePath = (char*)calloc(1024, sizeof(char));
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
            if(recvString(data->connection_fd, buffer, BUFFER_SIZE) == 0){//
                printf("[%d] Client disconnected!\n", (int)pthread_self());//if the client disconnected end thread, else, recive message.
                break;
            }


            sscanf(buffer, "%i %s %i", (int*)&(message.op),(message.password),&(message.account));//Get values from buffer, and assign them to variables.
            printf("opcion :%u\n", message.op);
            if(message.op == EXIT)//If the operation sent by the user is EXIT, end the thread.
            {// Client exit the program
                printf("[%d] Client is leaving!\n", (int)pthread_self());
                break;
            }
            if(interrupted){
                break;
            }
            if(message.op == VERIFY){//If the operation sent by the user is VERIFY, then the username and password are checked.
              if (!checkValidAccount(data,message.account,message.password))
              {//If the account is not valid, then send a message, else give access to the user, and notify him.
              sprintf(buffer, "%i",  INCORRECT);
              sendString(data->connection_fd, buffer);
              }else{
                sprintf(buffer, "%i",  CORRECT);
                sendString(data->connection_fd, buffer);
                out = 1;
                printf("[%d] Successful transaction!\n", (int)pthread_self());
                break;
              }

            }else{
              user = SaveUser(data, message.password);//Register the user and notify him.
              sprintf(buffer, "%i %i",  REGISTERS,user);
              sendString(data->connection_fd, buffer);
            }

            printf("[%d] Successful transaction!", (int)pthread_self());

          }

          if(recvString(data->connection_fd, buffer, BUFFER_SIZE) == 0){//if the client disconnected end thread, else, recive message.
              printf("[%d] Client disconnected!\n", (int)pthread_self());
              break;
          }

          sscanf(buffer, "%i %f %ld %s %s", (int*)&(message.op),&(message.value),&numbytes, (message.optionalFileName), message.optionalFilePath);//Get values from buffer, and assign them to variables.
          if(message.op == EXIT){//If the operation sent by the user is EXIT, end the thread.
              printf("[%d] Client is leaving!\n", (int)pthread_self());
              break;
          }

          switch(message.op){//Switch to make the server functions
              case RECEIVEFILE://Case to receive a File from the client and store it in it's directory.
                sprintf(buffer, "%i %ld",  CLIENTUPLOADFILE, numbytes);
                sendString(data->connection_fd, buffer);//Send a message to the client, leting it know that it can start sending the file.
                packet = (unsigned char*)calloc(fileMaxSize, sizeof(unsigned char));//Memory is allocated to the packet, with the maximum size of the packet.
                strcpy(buffer, message.optionalFilePath);
                strcat(buffer, message.optionalFileName);
                if(serverDownloadFile(numbytes, 0, 0, packet, data->connection_fd, buffer)){
                  printf("\nFile was uploaded correctly\n");
                }else{
                  printf("\nThere was a problem\n");
                }
                //When the file is received save the file in the user's directory, then release the packet's and the file's memory and assign the bytesTransfered and numbytes to 0.
                numbytes = 0;
                break;
              case SENDFILE://Case to send a file to the client(Downloading).
                numbytes = readFile(data, message.account, message.optionalFileName, message.optionalFilePath);//The file array is the result of the reading of the file that will be downloaded.
                packet = (unsigned char*)calloc(fileMaxSize, sizeof(unsigned char));
                sprintf(buffer, "%i %ld",  CLIENTRECIVEFILE, numbytes);//Send a message to the client, leting it know that the server is ready to send the file.
                sendString(data->connection_fd, buffer);
                strcpy(buffer, message.optionalFilePath);
                strcat(buffer, message.optionalFileName);
                if(serverUploadFile(numbytes, 0,packet, data->connection_fd, buffer)){
                  printf("\nFile was downloaded correctly\n");
                }else{
                  printf("\nThere was a problem\n");
                }
                numbytes = 0;
                free(packet);
                break;
              case GETDIRECTORIES://Get the files and directories of current folder.
                sprintf(buffer, "%i %d",  CLIENT_SHOW_DIRECTORIES, 10);
                sendString(data->connection_fd, buffer);//Let know the client that the server is about to send the result to the client.
                struct dirent *de;  // Pointer for directory entry
                DIR *dr = opendir(message.optionalFilePath);
                if (dr == NULL){  // opendir returns NULL if couldn't open directory
                    printf("Could not open current directory" );
                    return 0;
                  }
                  char * dirs = (char*)calloc(BUFFER_SIZE, sizeof(char));//Allocate memory for the string containing the result.
                  while ((de = readdir(dr)) != NULL){
                      strcat(de->d_name, " ");
                      strcat(dirs, de->d_name);
                  }

                  sprintf(buffer, "%s", dirs);
                  sendString(data->connection_fd, buffer);//Send the result to the client and release memory.
                  closedir(dr);
                  free(dirs);
                break;
              case CHECKA://check the balance
                  sprintf(buffer, "%i",  ASK_ACCOUNT);//send an error
                  sendString(data->connection_fd, buffer);
                    if(recvString(data->connection_fd, buffer, BUFFER_SIZE) == 0){
                        printf("[%d] Client disconnected!\n", (int)pthread_self());//if the client disconnected
                        break;
                    }
                  sscanf(buffer, "%s",&(message.other_account));//store the account that was asked
                  int result=checkPermissions(data ,message.account,&(message.other_account));//enter to the fucntion checkPermissions
                  if (result == message.account)//if the result was the same as the account number
                  {
                    sprintf(buffer, "%i",  FOUND);//send a found to the client
                    sendString(data->connection_fd, buffer);
                  }else{
                    sprintf(buffer, "%i",  NOFOUND);//send error
                    sendString(data->connection_fd, buffer);
                  }
                  break;
              case GIVE://check the balance
                sprintf(buffer, "%i",  ASK_ACCOUNT);//ask the account
                sendString(data->connection_fd, buffer);
                if(recvString(data->connection_fd, buffer, BUFFER_SIZE) == 0){
                    printf("[%d] Client disconnected!\n", (int)pthread_self());//if the client disconnected
                    break;
                }
                sscanf(buffer, "%s %s",&(message.other_account),message.optionalFilePath);//get the account and path
                AddPersmisses(data ,message.account,&message.other_account,message.optionalFilePath);//enter to function
                sprintf(buffer, "%i",  WRITTEN);//send a message
                sendString(data->connection_fd, buffer);
                  break;
              case CREATEDIRECTORY:// PENDING
                break;
              case REVOKE:
                  sprintf(buffer, "%i",  ASK_ACCOUNT);//ask the account
                  sendString(data->connection_fd, buffer);//send buffer
                  if(recvString(data->connection_fd, buffer, BUFFER_SIZE) == 0){//recive message
                      printf("[%d] Client disconnected!\n", (int)pthread_self());//if the client disconnected
                      break;
                  }
                  sscanf(buffer, "%s %s",&(message.other_account),message.optionalFilePath);//get the account and path
                  DeletePermissions(data ,message.account,&message.other_account);//enter to account
                  sprintf(buffer, "%i",  WRITTEN);//send a message
                  sendString(data->connection_fd, buffer);

                  break;
              case DELETE://check the balance
                sprintf(buffer, "%i",  NAMEFILE);//send an error
                sendString(data->connection_fd, buffer);
                if(recvString(data->connection_fd, buffer, BUFFER_SIZE) == 0){
                      printf("[%d] Client disconnected!\n", (int)pthread_self());//if the client disconnected
                        break;
                    }
                    sscanf(buffer, "%s %s", message.optionalFileName,message.optionalFilePath);//get the account and path
                    int status =  Delete(data, message.account,message.optionalFileName,message.optionalFilePath);//get the status of the file
                    if (status == 0)
                    {
                      sprintf(buffer, "%i",  OK);//send a ok message
                      sendString(data->connection_fd, buffer);
                    }
                    else
                    {
                      sprintf(buffer, "%i",  ERROR);//send a ok ERROR
                      sendString(data->connection_fd, buffer);

                    }
                    break;
              default:
                  fatalError("Unknown code!");//type a wrong option
          }
          printf("[%d] Successful transaction!\n", (int)pthread_self());
      }
  }
  sprintf(buffer, "%i %d",  BYE, 0);//finish the connection
  sendString(data->connection_fd, buffer);
  free(data);
  pthread_exit(NULL);
}
