/*
    Program for a blackjack game
    The  port needs to be provide as arguments to the program
    in the following format : ./server {port}

    Enrique Lira Martinez
    A01023351
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>

#include "sockets.h"

#define SERVICE_PORT 8642
#define BUFFER_SIZE 1024

void usage(char * program);
void connectToServer(char * address, char * port);
void communicationLoop(int connection_fd);

int main(int argc, char * argv[])
{
    printf("\n=== CLIENT PROGRAM ===\n");

    if (argc != 3)
        usage(argv[0]);

    connectToServer(argv[1], argv[2]);

    return 0;
}

// Show the user how to run this program
void usage(char * program)
{
    printf("Usage:\n%s {server_address} {port_number}\n", program);
    exit(EXIT_FAILURE);
}

// Establish a connection with the server indicated by the parameters
void connectToServer(char * address, char * port)
{
    struct addrinfo hints;
    struct addrinfo * server_info;
    int connection_fd;

    // Prepare the information to determine the local address
    bzero(&hints, sizeof hints);
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    ///// GETADDRINFO
    // Get the actual address of this computer
    if (getaddrinfo(address, port, &hints, &server_info) == -1)
    {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    ///// SOCKET
    // Use the information obtained by getaddrinfo
    connection_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (connection_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    ///// CONNECT
    // Connect with the desired port
    if (connect(connection_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // Release the memory of the list of addrinfo
    freeaddrinfo(server_info);

    // Establish the communication
    communicationLoop(connection_fd);

    // Close the socket
    close(connection_fd);
}

// Do the actual receiving and sending of data
void communicationLoop(int connection_fd)
{

    char buffer[BUFFER_SIZE];//variable to exchange data to the server
    int chars_read;
    int action = 0;//varaible to store the action to follow
    int finish=0;//varaible to finish the program
    int bet;

    printf("Welcome to Black Jack Online \n do you wish to play? (yes = 1/no = 2)");
    scanf("%d", &action);//ask the client for a game
    if(action == 1)
    {
      sprintf(buffer, "%d", action);//send the action to the server
      sendMessage(connection_fd, buffer, strlen(buffer));

      printf("Enter your bet: (int)\n");//ask for the bet
      scanf("%d", &bet);
      sprintf(buffer, "%d", bet);//store in the buffer
      sendMessage(connection_fd, buffer, strlen(buffer));//send the bet to the server
    }
    else if (action == 2)//exit
    {
      printf("Quitting game!\n");
      return;
    }
    else
    {
      printf("Invadlid option\n");//if the client fails
      return;
    }

    receiveMessage(connection_fd, buffer, BUFFER_SIZE);//recive the cards
    printf("These are your cards are: %s \n", buffer);


    while (finish==0)
    {
      if (strncmp(buffer, "LOST", 4) == 0)// if the buffer is a LOST
      {
            printf("YOU LOST\n");
          finish=1;
      }
      else if(strncmp(buffer, "WIN", 3) == 0)// if the buffer is a WIN
      {
        printf("YOU WON!\n");
        finish = 1;
      }
      else if(strncmp(buffer, "TIE", 3) == 0)// if the buffer is a TIE
      {
          printf("IT IS A TIE!\n");
          finish = 1;
      }
      else
      {
        printf("\nWhat do you want to do? \n1.HIT\n2.STAND\n3.EXIT:");//ask for action
        scanf("%d", &action);
        sprintf(buffer, "%d", action);//store in the buffer
        sendMessage(connection_fd, buffer, strlen(buffer));//send the action
        if(action == 1)//if there a 1 means a hit
        {
        chars_read = receiveMessage(connection_fd, buffer, BUFFER_SIZE);// recive the new card and the previous hand
        printf("This are your cards: %s\n", buffer);
        }
        else if (action == 2)//if there a 1 means a stand
        {
          chars_read = receiveMessage(connection_fd, buffer, BUFFER_SIZE);// recive the cards from the dealer
          printf("\nThese are the dealer cards: %s \n", buffer);
          chars_read = receiveMessage(connection_fd, buffer, BUFFER_SIZE);//recive the final result (WIN,LOST,TIE)

        }
      }
    }
    sprintf(buffer, "BYE");
    sendMessage(connection_fd, buffer, strlen(buffer));
    chars_read = receiveMessage(connection_fd, buffer, BUFFER_SIZE);
}
