/*
    Library with common sockets functions

    Enrique Lira Martinez
    A01023351
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include "sockets.h"

#define MAX_QUEUE 5


// Receive a message and validate the return values
int receiveMessage(int connection_fd, char * buffer, int buff_length)
{
    int chars_read;

    // Clear the buffer
    bzero(buffer, buff_length);
    // Get an incomming message
    chars_read = recv(connection_fd, buffer, buff_length, 0);
    if (chars_read == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    if (chars_read == 0)
    {
        printf("Client disconnected\n");
    }

    return chars_read;
}

// Send a message and validate
void sendMessage(int connection_fd, char * buffer, int buff_length)
{
    if (send(connection_fd, buffer, strlen(buffer), 0) == -1)
    {
        perror("send");
        exit(EXIT_FAILURE);
    }

}

// Initialize the server to be ready for connections
int startServer(char * port)
{
    struct addrinfo hints;
    struct addrinfo * server_info = NULL;
    int server_fd;

    // Prepare the information to determine the local address
    // Clear the structure
    bzero(&hints, sizeof hints);
    // Let the IP address be found automatically
    hints.ai_flags = AI_PASSIVE;
    // Use IPv4 address
    hints.ai_family = AF_INET;
    // Type of socket
    hints.ai_socktype = SOCK_STREAM;

    ///// GETADDRINFO
    // Get the actual address of this computer
    if ( getaddrinfo(NULL, port, &hints, &server_info) == -1)
    {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    ///// SOCKET
    // Use the information obtained by getaddrinfo
    server_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (server_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Allow using the same port immediately, even if the previous server did not close correctly


    ///// BIND
    // Connect with the desired port
    if (bind(server_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Release the memory of the list of addrinfo
    freeaddrinfo(server_info);

    ///// LISTEN
    // Prepare for connections
    if (listen(server_fd, MAX_QUEUE) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}
