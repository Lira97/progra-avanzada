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
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
// Custom libraries

#include <sys/poll.h>

#include "bank_codes.h"
#include "sockets.h"
#include "fatal_error.h"

#define BUFFER_SIZE 15000

///// FUNCTION DECLARATIONS
void usage(char * program);
void bankOperations(int connection_fd);

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int connection_fd;

    printf("\n=== BANK CLIENT PROGRAM ===\n");

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
void bankOperations(int connection_fd)
{
    char * fileName = NULL;
    fileName = (char*)calloc(255, sizeof(char));
    char buffer[BUFFER_SIZE];
    int bytesTransfered = 0;
    char password[255];
    int account;
    char other_account;
    int second_option=0;
    int user;
    float balance;
    char option = 'p';
    char result = 'c';
    int status = 10;
    operation_t operation;

    FILE   *infile;
    unsigned char *message = NULL;
    long numbytes =0;

    struct pollfd stdin_fds[1];
    struct pollfd server_fds[1];
    int timeout = 10; // 10ms tiemout
    int poll_stdin;
    int poll_server;
    while (option != 'w')
    {
      printf("Bank login:\n");
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
        switch(option)
        {
            case 'c':
                printf("Enter account ");
                scanf("%d", &account);
                printf("Enter password ");
                scanf("%s", password);
                operation = VERIFY;
                break;
            // Deposit into account
            case 'r':
                printf("Enter password ");
                scanf("%s", password);
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
        sprintf(buffer, "%d %s %d", operation, password, account);
        sendString(connection_fd, buffer);
//        sendBinary(connection_fd, buffer);
        // Receive the response
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Server closed the connection\n");
            break;
        }
        // get the data
        sscanf(buffer, "%d %d", &status, &user);
          switch (status)
          {
            case CORRECT:
                printf("Welcome to the NAS system\n");
                option = 'w';
                break;
            case REGISTERS:
                printf("Your registration was made successfully\n");
                printf("you are the user number : %d\n", user);
                break;
            case INCORRECT:
                printf("\tIcorrect password or account\n");
                break;

        }

    }
    operation = 0;
    status =0;
    while (option != 'x')
     {
        printf("Bank menu:\n");
        printf("Select an option: \n");
        printf("\tc. Check balance\n");
        printf("\ts. SEND\n");
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

        switch(option){
            // Check balance
            case 'c':
                printf("Enter account: ");
                scanf("%d", &account);
                operation = RECEIVEFILE;
                break;
            // Deposit into account
            case 's':
                printf("Enter File Name ");
                scanf("%s", fileName);
                printf("%s\n",fileName);
                infile = fopen(fileName, "rb");

                /* quit if the file does not exist */
                if(infile == NULL)
                    return;

                /* Get the number of bytes */
                fseek(infile, 0L, SEEK_END);
                numbytes = ftell(infile);

                /* reset the file position indicator to
                the beginning of the file */
                fseek(infile, 0L, SEEK_SET);

                /* grab sufficient memory for the
                buffer to hold the text */
                //                message = (char*)calloc(numbytes, sizeof(char));
                message = (unsigned char*)calloc(numbytes, sizeof(unsigned char));
                /* memory error */
                if(message == NULL)
                    return;

                /* copy all the text into the buffer */
                fread(message, sizeof(char), numbytes, infile);
                fclose(infile);
                operation = RECEIVEFILE;
                sprintf(buffer, "%d %f %ld %s %s", operation, balance,numbytes, fileName, "ServerFiles/");
                break;
            // Exit the bank
                case 'l':
                operation = GETDIRECTORIES;
                sprintf(buffer, "%d", operation);

                break;
            // Exit the bank
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


        // Prepare buffer to send

        // Send the messag
        sendString(connection_fd, buffer);
//        sendBinary(connection_fd, buffer);
        // Receive the response
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Server closed the connection\n");
            break;
        }
        // get the data
        sscanf(buffer, "%d %f", &status, &balance);

        int maxSize = BUFFER_SIZE;
        int success = 0;
        if(maxSize > numbytes){
            maxSize = numbytes;
        }
        unsigned char * packet = (unsigned char*)calloc(BUFFER_SIZE, sizeof(unsigned char));

        printf("operation: %u %u \n",operation, status);
        switch (status){
            case OK:
                printf("\tThe balance in account %d is %.2f\n", account, balance);
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
            case ASK:

            while (bytesTransfered < numbytes){
                if(success == 0){
                    for(int i = 0; i < maxSize; i++){
                        if(bytesTransfered >= numbytes){
                            break;
                        }
                        packet[i] = message[bytesTransfered];
                        bytesTransfered++;
                    }
                    success = 1;
                }
//                        int progress = (bytesTransfered*100)/numbytes;
                write(connection_fd, packet, maxSize);
                read(connection_fd, buffer, 1);
                sscanf(buffer, "%d", &success);
                printf("\rUploading File, Please Wait %d MB OF %ld", bytesTransfered, numbytes);
                fflush(stdout);
            }
//                write(connection_fd, buffer, 0);
                free(packet);
                printf("\n");
                break;
            case INSUFFICIENT:
                recvString(connection_fd, buffer, BUFFER_SIZE);
                char * dirs = (char*)calloc(5000, sizeof(char));
                sscanf(buffer, "%s", dirs);
                printf("\t%s\n", buffer);
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
