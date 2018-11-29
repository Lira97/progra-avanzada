/*
    Enrique Lira Martinez A01023351
    For this part of the program i used the help of my partner Emiliano Abascal Gurria
    Client program to access the accounts in the bank
    This program connects to the server using sockets

    Gilberto Echeverria
    gilecheverria@yahoo.com
    29/03/2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
// Custom libraries

#include <sys/poll.h>
#include "codes.h"
#include "client.h"
#include "sockets.h"
#include "fatal_error.h"



///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int connection_fd;

    printf("\n=== NAS CLIENT PROGRAM ===\n");

    // Check the correct arguments
    if (argc != 3)
    {
        usage(argv[0]);
    }

    // Start the server
    connection_fd = connectSocket(argv[1], argv[2]);
	// Use the bank operations available
    bankOperations(connection_fd);
    // Close the socket
    close(connection_fd);

    return 0;
}

///// FUNCTION DEFINITIONS

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {server_address} {port_number}\n", program);
    exit(EXIT_FAILURE);
}

/*
    Main menu with the options available to the user
*/
void bankOperations(int connection_fd){
    //Variable declaration
    int success = 0;
    int bytesTransfered = 0;
    int fileMaxSize = 1024;
    int second_option=0;
    int status = 10;
    char other_account;
    char option = 'p';
    char buffer[BUFFER_SIZE];
    char * fileName = NULL;
    float balance;
    long numbytes =0;
    userStruct user;
    operation_t operation;
    unsigned char *fileToTransfer, *packet = NULL;
    FILE *infile;
    struct pollfd stdin_fds[1];
    struct pollfd server_fds[1];
    int timeout = 10; // 10ms tiemout
    int poll_stdin;
    int poll_server;
    while (option != 'w')
    {
      printf("Login:\n");
      printf("Select an option: \n");
      printf("\tc. Access account\n");
      printf("\tr. Register account\n");
      printf("\tx. Exit program\n");
    while(1){
            //this is a double poll, with this the client can be notice when the server had shut down.
            stdin_fds[0].fd = stdin->_file;// Poll for the input from the user
            stdin_fds[0].events = POLLIN;
            server_fds[0].fd = connection_fd;//this poll is for the server to listen a signal from it
            server_fds[0].events = POLLIN;

            poll_stdin = poll(stdin_fds, 1, timeout);
            if(poll_stdin == -1){
                fatalError("POLL");
            }
            else if(poll_stdin == 0){
                poll_server = poll(server_fds, 1, timeout);
                if(poll_server == -1){
                    fatalError("SERVER POLL");
                }else if(poll_server == 0){
                    // Nothing
                    continue;
                }else{
                    // Receive the response
                    if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ){
                        printf("\nServer finich conection\n");
                        exit(0);
                    }
                    // Get the data
                    sscanf(buffer, "%d %f", &status, &balance);

                    if(status == BYE){
                        printf("\nSHUTTING DOWN!\n");
                        exit(0);
                    }
                }
                continue;
            }else{
              scanf("%c", &option);//get the client opction
              break;
            }
        }
        user.password = (char *)calloc(255, sizeof(char *));
        switch(option)
        {
            case 'c':
                printf("Enter account: ");
                scanf("%d", &user.account);
                printf("Enter password: ");
                scanf("%s", user.password);
                operation = VERIFY;
                break;
            // Deposit into account
            case 'r':
                printf("Enter password ");
                scanf("%s", user.password);
                operation = REGISTER;
                break;
            // Deposit into account
            case 'x':
                printf("Thanks for using the program. Bye!\n");
                break;
            default:
                printf("Invalid option. Try again ...\n");
                // Skip the rest of the code in the while
                continue;
        }
        if(option == 'x')
        break;
        sprintf(buffer, "%d %s %d", operation, user.password, user.account);
        sendString(connection_fd, buffer);
        // Receive the response
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Server closed the connection\n");
            break;
        }
        // get the data
        sscanf(buffer, "%d %d", &status, &user.account);
        
          switch (status)
          {
            case CORRECT:
                printf("Welcome to the NAS system\n");
                option = 'w';
                break;
            case REGISTERS:
                printf("Your registration was made successfully\n");
                printf("you are the user number : %d\n", user.account);
                break;
            case INCORRECT:
                printf("\tIcorrect password or account\n");
                break;
        }

    }
    operation = 0;
    status =0;
    user.homeDirectory = (char *)calloc(10, sizeof(char *));
    sprintf(user.homeDirectory, "ServerFiles/%d/", user.account);
    while (option != 'x')
     {
        printf("\nUSER: %d\n", user.account);
        printf("Menu:\n");
        printf("Select an option: \n");
        printf("\tc. Check balance\n");
        printf("\tu. Upload File\n");
        printf("\td. Download File\n");
        printf("\tl. Get Directories\n");
        printf("\tq. Check permisses\n");
        printf("\tg. Give permisses\n");
        printf("\tx. Exit program\n");

    while(1){
            //this is a double poll, with this the client can be notice when the server had shut down.
            stdin_fds[0].fd = stdin->_file;// Poll for the input from the user
            stdin_fds[0].events = POLLIN;
            server_fds[0].fd = connection_fd;//this poll is for the server to listen a signal from it
            server_fds[0].events = POLLIN;

            poll_stdin = poll(stdin_fds, 1, timeout);
            if(poll_stdin == -1){
                fatalError("POLL");
            }
            else if(poll_stdin == 0){
                poll_server = poll(server_fds, 1, timeout);
                if(poll_server == -1){
                    fatalError("SERVER POLL");
                }else if(poll_server == 0){
                    // Nothing
                    continue;
                }else{
                    // Receive the response
                    if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ){
                        printf("\nServer finich conection\n");
                        exit(0);
                    }
                    // Get the data
                    sscanf(buffer, "%d %f", &status, &balance);

                    if(status == BYE){
                        printf("\nSHUTTING DOWN!\n");
                        exit(0);
                    }
                }
                continue;
            }else{
                scanf(" %c", &option);//get the client opction
                break;
            }
        }

        // Init variables to default values

        balance = 0;
        bytesTransfered = 0;

        switch(option){//Switch to add menu functionality.
            // Check balance
            case 'c'://If the
                printf("Enter account: ");
                scanf("%d", &user.account);
                operation = RECEIVEFILE;
                break;
            // Deposit into account
            case 'u'://Case to upload files
                fileName = (char*)calloc(255, sizeof(char));//Memory allocation for the file name.
                printf("Enter File Name ");//Scan for the file name.
                scanf("%s", fileName);
                infile = fopen(fileName, "rb");//Open the file that the client will upload.
                /* quit if the file does not exist */
                if(infile == NULL)
                    return;

                //Get the numner of bytes
                fseek(infile, 0L, SEEK_END);
                numbytes = ftell(infile);
                fseek(infile, 0L, SEEK_SET);

                //Assign memory for the variable the will contain the file bytes.
                fileToTransfer = (unsigned char*)calloc(numbytes, sizeof(unsigned char));
                /* memory error */
                if(fileToTransfer == NULL)
                    return;

                fread(fileToTransfer, sizeof(char), numbytes, infile);//Assign the file info to the fileToTransfer variable.
                fclose(infile);//Close the file.
                operation = RECEIVEFILE;//Set the operation that the client wants the server to do.
                sprintf(buffer, "%d %f %ld %s %s", operation, balance,numbytes, fileName, user.homeDirectory);//Parse the variables into the buffer.
                free(fileName);//Release the memory for the file name.
                break;
            // Exit the bank
            case 'l'://Case to show the files in the home directory of the user.
                operation = GETDIRECTORIES;//Set the operation that the client wants the server to do.
                sprintf(buffer, "%d %f %ld %s %s", operation, 0.0, numbytes, "|", user.homeDirectory);//Parse the variables into the buffer.
                break;
            case 'd':
              operation = SENDFILE;//Set the operation that the client wants the server to do.
              fileName = (char*)calloc(255, sizeof(char));//allocate memory for the file name and get it from the user.
              printf("Enter File Name: ");
              scanf("%s", fileName);
              sprintf(buffer, "%d %f %ld %s %s", operation, balance,numbytes, fileName, user.homeDirectory);//Parse the variables into the buffer.
              free(fileName);
              break;
            case 'q':
                operation = CHECKA;
                sprintf(buffer, "%d", operation);
                break;
            // Exit the bank
            case 'g':
                operation = GIVE;
                sprintf(buffer, "%d", operation);
                break;
            // Exit the bank
            case 'x':
                printf("Thanks for using the program. Bye!\n");
                operation = EXIT;
                sprintf(buffer, "%d", operation);
                break;
            // Incorrect option
            default:
                printf("Invalid option. Try again ...\n");
                // Skip the rest of the code in the while
                continue;
        }


        sendString(connection_fd, buffer);//Send the buffer to the server.
        // Receive the response
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )//Receive the server's response to know what to to next.
        {
            printf("Server closed the connection\n");
            break;
        }
        // get the data
        sscanf(buffer, "%d %ld", &status, &numbytes);

        
        
        
        printf("operation: %u %u \n",operation, status);
        switch (status){//Switch to know what operation to do in the client's side.
            case OK:
                printf("\tThe balance in account %d is %.2f\n", user.account, balance);
                break;
            case ASK_ACCOUNT:
                printf("Enter account: ");
                scanf("%s", &other_account);
                sprintf(buffer, "%s", &other_account);
                sendString(connection_fd, buffer);
                recvString(connection_fd, buffer, BUFFER_SIZE);
                sscanf(buffer, "%i",&second_option);
                if(second_option == WRITTEN)
                {
                  printf("\tSuccess\n");
                  break;
                }
                else
                {
                  if (second_option == FOUND)
                  printf("acccesss\n" );
                  else
                  printf("noooo\n" );
                  break;
                }
            case CLIENTUPLOADFILE://Case to upload file to the server.
              if(fileMaxSize > numbytes){//If the size of the maximum size of the packet is greater than the bytes that the file to be uploaded has, then the maximum size will be the number of bytes of the file, this is so that if the file is very small, then it can be sent in only one packet.
                fileMaxSize = numbytes;
              }
              while (bytesTransfered < numbytes){//While the bytes sent by the server are lower than the total number of bytes that the file is made of, then if success is set to 0, then iterate from 0 to the maximum size of each packet, and assign each byte to the corresponding index of the file array that the server is about to send, set the success var to one, and send to the server the packet, the success variable is used so that if the server is done receiving the packet, then it tells the server to send the next one, this is done because with some files the client would not be done receiving each packet when the server would send another one, then print the download process.
                if(bytesTransfered == 0){
                  packet = (unsigned char*)calloc(fileMaxSize, sizeof(unsigned char));
                }
                  if(success == 0){
                      for(int i = 0; i < fileMaxSize; i++){
                          if(bytesTransfered >= numbytes){
                              break;
                          }
                          packet[i] = fileToTransfer[bytesTransfered];
                          bytesTransfered++;
                      }
                      success = 1;
                  }
                  write(connection_fd, packet, fileMaxSize);
                  read(connection_fd, buffer, 1);
                  sscanf(buffer, "%d", &success);
                  printf("\rUploading File, Please Wait %d bytes OF %ld", bytesTransfered, numbytes);
                  fflush(stdout);
              }
                free(packet);//Release the packet and fileToTransfer memory and set usefull variables to 0.
                free(fileToTransfer);
                success = 0;
                numbytes = 0;
                bytesTransfered = 0;
                fileMaxSize = 1024;
                break;
                
            case CLIENTRECIVEFILE://Case to Download files from the server.
                if(fileMaxSize > numbytes){//If the size of the maximum size of the packet is greater than the bytes that the file to be uploaded has, then the maximum size will be the number of bytes of the file, this is so that if the file is very small, then it can be sent in only one packet.
                  fileMaxSize = numbytes;
                }
                success = 0;
                sprintf(buffer, "%d", success);
                sendString(connection_fd, buffer);//Tell the server that the client is ready to start receiving the file.
                
                fileToTransfer = (unsigned char*)calloc(numbytes, sizeof(unsigned char));
                packet = (unsigned char*)calloc(fileMaxSize, sizeof(unsigned char));
                while(numbytes > bytesTransfered){//While the bytes sent by the client are lower than the total number of bytes that the file is made of, then read the packet sent from the server, iterate from 0 to the maximum size of each packet, and assign each byte to the corresponding index of the file array that it's receiving, print the state of the upload, and tell to the server that the server is ready to receive the next packet(set of bytes).
                  read(connection_fd, packet, fileMaxSize);
                    for(int i = 0; i < fileMaxSize; i++){
                      if(bytesTransfered >= numbytes){
                        break;
                      }
                      fileToTransfer[bytesTransfered] = packet[i];
                      bytesTransfered++;
                    }
                  printf("\rFile Download In progress %d bytes of %ld", bytesTransfered, numbytes);
                  fflush(stdout);
                  write(connection_fd, "0", 1);

                }
              
              if(saveFile(fileToTransfer, fileName, infile, numbytes)){//Save the fully assambled file.
                printf("\nFile Downloaded Succesfully\n");
              }else{
                printf("\nSomething went wrong\n");
              }
              free(fileToTransfer);//Release the packet and fileToTransfer memory and set usefull variables to 0.
              free(packet);
              bytesTransfered = 0;
              numbytes = 0;
              fileMaxSize = 1024;
              break;
            case CLIENT_SHOW_DIRECTORIES://See the files and directory of a directory.
                recvString(connection_fd, buffer, BUFFER_SIZE);//Receive the buffer.
                char * dirs = (char*)calloc(255, sizeof(char));
                sscanf(buffer, "%s", dirs);
                printf("\t%s\n", buffer);//Print the directories sent from the server.
                free(dirs);//Release memory.
                break;
            case BYE:
                printf("\tThanks for connecting to the bank. Good bye!\n");
                break;
            case ERROR: default:
                printf("\tInvalid operation. Try again\n");
                break;
        }

    }
}

bool saveFile(unsigned char * file, char * fileName, FILE * infile, int numbytes){//Function to save files.
  infile = fopen(fileName, "wb");//Open the file.
  if(infile == NULL)//If file is not fount return false.
    return false;

  if(fwrite (file , sizeof(char),numbytes, infile)){//Copy all the file's content into the resulting file and return true.
    fclose(infile);
    return true;
  }else{//If there was an error return false.
    fclose(infile);
    return false;
  }
}
