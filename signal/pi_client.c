/*
Enrique Lira Martinez
A01022351
    Client program to get the value of PI
    This program connects to the server using sockets

    Gilberto Echeverria
    gilecheverria@yahoo.com
    21/02/2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <signal.h>
#include <errno.h>
// Custom libraries
#include "fatal_error.h"

#include <ifaddrs.h>

#define BUFFER_SIZE 1024

///// FUNCTION DECLARATIONS
int interrupted = 0;
void usage(char * program);
int openSocket(char * address, char * port);
void requestPI(int connection_fd);
void requestQuit(int server_fd);
void setupHandlers();
void detectInterruption(int signal);

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
  setupHandlers();
    int connection_fd;

    printf("\n=== CLIENT FOR COMPUTING THE VALUE OF pi ===\n");

    // Check the correct arguments
    if (argc != 3)
    {
        usage(argv[0]);
    }

    // Start the server
    connection_fd = openSocket(argv[1], argv[2]);
	// Listen for connections from the clients
    requestPI(connection_fd);
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
    Open the socket to the server
    Returns the file descriptor for the socket
    Remember to close the socket when finished
*/
int openSocket(char * address, char * port)
{
    struct addrinfo hints;
    struct addrinfo * server_info = NULL;
    int connection_fd;

    // Prepare the hints structure
    // Clear the structure for the server configuration
    bzero(&hints, sizeof hints);
    // Set to use IPV4
    hints.ai_family = AF_INET;
    // Use stream sockets
    hints.ai_socktype = SOCK_STREAM;
    // Get the local IP address automatically
    hints.ai_flags = AI_PASSIVE;

    // GETADDRINFO
    // Use the presets to get the actual information for the socket
    // The result is stored in 'server_info'
    if (getaddrinfo(address, port, &hints, &server_info) == -1)
    {
        fatalError("getaddrinfo");
    }

    // SOCKET
    // Open the socket using the information obtained
    connection_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (connection_fd == -1)
    {
        close(connection_fd);
        fatalError("socket");
    }

    // CONNECT
    // Connect to the server
    if (connect(connection_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        fatalError("connect");
    }

    // FREEADDRINFO
    // Free the memory used for the address info
    freeaddrinfo(server_info);

    return connection_fd;
}

void requestPI(int connection_fd)
{
    char buffer[BUFFER_SIZE];
    int chars_read;
    unsigned long int iterations;
    double result;
    // Variables for polling
    struct pollfd test_fds;
    int poll_result;
    int timeout = 10;

    printf("Enter the number of iterations for PI: ");
    scanf("%lu", &iterations);

    // Prepare the response to the client
    sprintf(buffer, "%lu", iterations);
    // SEND
    // Send the response
    if (send(connection_fd, buffer, strlen(buffer)+1, 0) == -1)
    {
        fatalError("send");
    }
    // Fill in the data for the structure
    test_fds.fd = connection_fd;// Polling stdin
    test_fds.events = POLLIN;

    // Compute the value of PI
    while (!interrupted)
    {

        poll_result = poll(&test_fds, 1, timeout);
        // Timeout finished without incidents
        if (poll_result == 0)
        {
        }
        // When the input is ready to be read, do the scanf and add the total
        else if (poll_result > 0)
        {
            if (test_fds.revents  & POLLIN)// Server sent back a response
            {
                break;
            }
        }
        // The poll returned an error
        else // if (poll_result == -1)
        {
            // If poll detected an error due to an interruption, exit the loop
            if (errno == EINTR)
            {
              //if there is a exit singal ,it  request the current value and break
              requestQuit(connection_fd);
                break;
            }
        }
    }

    // Clear the buffer
    bzero(buffer, BUFFER_SIZE);

    // RECV
    // Receive the request
    chars_read = recv(connection_fd, buffer, BUFFER_SIZE, 0);
    if (chars_read == -1)
    {
        fatalError("recv");
    }
    sscanf(buffer, "%lf", &result);

    // Print the result
    printf("The value for PI is: %.20lf\n", result);
}

void requestQuit(int server_fd)//fucntion to send a result to the server
{
    if (send(server_fd, "q", 1 + 1, 0) == -1 )
    {
        fatalError("send");
    }
}

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
