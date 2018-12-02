/*
    Enrique Lira Martinez A01023351
    For this part of the program use the help of my partner Emiliano Abascal Gurria
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

#define BUFFER_SIZE 1024

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
    char buffer[BUFFER_SIZE];
    int account;
    float amount;
    float balance;
    char option = 'c';
    int status;
    operation_t operation;

    struct pollfd stdin_fds[1];
    struct pollfd server_fds[1];
    int timeout = 10; // 10ms tiemout
    int poll_stdin;
    int poll_server;

    while (option != 'x')
    {
        printf("Bank menu:\n");
        printf("\tc. Check balance\n");
        printf("\td. Deposit into account\n");
        printf("\tw. Withdraw from account\n");
        printf("\tx. Exit program\n");
        printf("Select an option: ");


    while(1)
    {
            //this is a double poll, with this the client can be notice when the server had shut down.
            stdin_fds[0].fd = stdin->_file;// Poll for the input from the user
            stdin_fds[0].events = POLLIN;
            server_fds[0].fd = connection_fd;//this poll is for the server to listen a signal from it
            server_fds[0].events = POLLIN;

            poll_stdin = poll(stdin_fds, 1, timeout);
            if(poll_stdin == -1)
            {
                fatalError("POLL");
            }
            else if(poll_stdin == 0)
            {

                poll_server = poll(server_fds, 1, timeout);
                if(poll_server == -1)
                {
                    fatalError("SERVER POLL");
                }
                else if(poll_server == 0)
                {
                    // Nothing
                    continue;
                }
                else
                {
                    // Receive the response
                    if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
                    {
                        printf("\nServer finich conection\n");
                        exit(0);
                    }
                    // Get the data
                    sscanf(buffer, "%d %f", &status, &balance);
                    if(status == BYE)
                    {
                        printf("\nSHUTTING DOWN!\n");
                        exit(0);
                    }
                }
                continue;
            }
            else
            {
                scanf(" %c", &option);//get the client opction
                break;
            }
        }

        // Init variables to default values
        account = 0;
        amount = 0;
        balance = 0;

        switch(option)
        {
            // Check balance
            case 'c':
                printf("Enter account: ");
                scanf("%d", &account);
                operation = CHECK;
                break;
            // Deposit into account
            case 'd':
                printf("Enter account: ");
                scanf("%d", &account);
                printf("Enter the amount to deposit: ");
                scanf("%f", &amount);
                operation = DEPOSIT;
                break;
            // Withdraw from account
            case 'w':
                printf("Enter account: ");
                scanf("%d", &account);
                printf("Enter the amount to deposit: ");
                scanf("%f", &amount);
                operation = WITHDRAW;
                break;
            // Exit the bank
            case 'x':
                printf("Thanks for using the program. Bye!\n");
                operation = EXIT;
                break;
            // Incorrect option
            default:
                printf("Invalid option. Try again ...\n");
                // Skip the rest of the code in the while
                continue;
        }

        // Prepare buffer to send
        sprintf(buffer, "%d %d %f", operation, account, amount);

        // Send the message
        sendString(connection_fd, buffer);

        // Receive the response
        if ( !recvString(connection_fd, buffer, BUFFER_SIZE) )
        {
            printf("Server closed the connection\n");
            break;
        }
        // get the data
        sscanf(buffer, "%d %f", &status, &balance);

        // Print the result
        switch (status)
        {
            case OK:
                printf("\tThe balance in account %d is %.2f\n", account, balance);
                break;
            case INSUFFICIENT:
                printf("\tInsufficient funds for the transaction selected\n");
                break;
            case NO_ACCOUNT:
                printf("\tInvalid acount number entered\n");
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
