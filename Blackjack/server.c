/*
    Program for a simple chat server
    Can only connect with one client at a time
    The client address and port are provided as arguments to the program
    in the following format : ./client {ip} {port}

    Enrique Lira Martinez
    A01023351
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

// Include libraries for sockets
#include <netdb.h>
#include <arpa/inet.h>

// My sockets library
#include "sockets.h"

#define SERVICE_PORT 8642
#define MAX_QUEUE 5
#define BUFFER_SIZE 1023

void usage(char * program);
void waitForConnections(int server_fd);
void communicationLoop(int connection_fd);
int randomCard();

int main(int argc, char * argv[])
{
    int server_fd;

    printf("\n=== SERVER PROGRAM ===\n");

    if (argc != 2)
        usage(argv[0]);

    server_fd = startServer(argv[1]);

    printf("Server ready and waiting!\n");
    // Stand by to receive connections from the clients
    waitForConnections(server_fd);

    printf("Closing the server socket\n");
    // Close the socket
    close(server_fd);

    return 0;
}

// Show the user how to run this program
void usage(char * program)
{
    printf("Usage:\n%s {port_number}\n", program);
    exit(EXIT_FAILURE);
}



// Stand by for connections by the clients
void waitForConnections(int server_fd)
{
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int connection_fd;
    pid_t pid;

    // Loop to wait for client connections
    while (1)
    {
        ///// ACCEPT
        // Receive incomming connections
        // Get the size of the structure where the address of the client will be stored
        client_address_size = sizeof client_address;
        connection_fd = accept(server_fd, (struct sockaddr *) &client_address, &client_address_size);
        if (connection_fd == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Identify the client
        // Get the ip address from the structure filled by accept
        inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
        printf("Received connection from '%s' on port '%d'\n", client_presentation, client_address.sin_port);

        pid = fork();
        // Parent process
        if (pid > 0)
        {
            close(connection_fd);
        }
        // Child process
        else if (pid == 0)
        {
            close(server_fd);
            // Establish the communication
            communicationLoop(connection_fd);
            exit(EXIT_SUCCESS);
        }
        // Fork error
        else
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

    }
}

// Do the actual receiving and sending of data
void communicationLoop(int connection_fd)
{
  srand(time(NULL));
  char buffer[BUFFER_SIZE];//variable to exchange data to the server
  char temp[BUFFER_SIZE];//variable to copy the buffer information
  int action=0;//varaible to store the action to follow
  int finish=0;//varaible to finish the program

  hand_t hand;// struct for the client hand
  hand.num = 0;
  hand.sum = 0;

  hand_t dealer;// struct for the server hand
  dealer.num = 0;
  dealer.sum = 0;
  printf("Game will start, generating random cards\n");

  receiveMessage(connection_fd, buffer, BUFFER_SIZE);
  if(strncmp(buffer, "1", 1) == 0)
  {
       hand.cards[hand.num++] = randomCard();//sum a car form the deck to the client hand
       hand.sum += hand.cards[0];//sum the current card value
       hand.cards[hand.num++] = randomCard();//sum a car form the deck to the client hand
       hand.sum += hand.cards[1];//sum the current card value
       dealer.cards[dealer.num++] = randomCard();//sum a car form the deck to the server hand
       dealer.sum += dealer.cards[0];//sum the current card value
       dealer.cards[dealer.num++] = randomCard();//sum a car form the deck to the server hand
       dealer.sum += dealer.cards[1];//sum the current card value
       printf("client : %d %d\n", hand.cards[0], hand.cards[1]);
       printf("server : %d %d\n", dealer.cards[0], dealer.cards[1]);
       receiveMessage(connection_fd, buffer, BUFFER_SIZE);//recive the bet from the client
       sscanf(buffer, "%d", &hand.bet);//store the bet

    }
   sprintf(buffer, "%d %d sum : %d \nDealer card %d", hand.cards[0],hand.cards[1],hand.sum,dealer.cards[0]);
   sendMessage(connection_fd, buffer, strlen(buffer));// send the cards for the client and one card from the server

    while (finish == 0)//it wiil break until the value get a 1
    {
      receiveMessage(connection_fd, buffer, BUFFER_SIZE);// recives the action
      sscanf(buffer, "%d", &action);

      if (action == 1)// if action is 1 ,it  means HIT
      {

        int card = randomCard();//store the value of a card
        hand.cards[hand.num++] = card;//sum a car form the deck to the client hand
        hand.sum += card;//sum the current card value
        sprintf(buffer, "");//clean buffer
        if(hand.sum > 21)// if sum is higher than 21 you lose
        {
          sprintf(buffer, "LOST");//store char lost to the buffer
          finish = 1;//finish the loop
        }
        for(int j = 0; j < hand.num; j++)
        {
          sprintf(temp, " %d  ", hand.cards[j]);//store all client  hand in temp
          strcat(buffer, temp);// it concat in the buffer
        }
        sprintf(temp, " sum : %d \nDealer card %d ",  hand.sum, dealer.cards[0]);
        strcat(buffer, temp);
        sendMessage(connection_fd, buffer, strlen(buffer));

      }
      else if(action == 2)// if action is 1 ,it  means STAND
      {
          while (dealer.sum < 17)//soft 17
          {
            int card = randomCard(); //store the value of a card
            dealer.cards[dealer.num++] = card;//sum a car form the deck to the client hand
            dealer.sum += card;//sum the current card value

          }
          sprintf(buffer, "");//clean buffer
          printf("server :");
          for(int j = 0; j < dealer.num; j++)//store all server hand in temp
          {
            printf("%d  ", dealer.cards[j]);
            sprintf(temp, " %d  ", dealer.cards[j]);//store all client  hand in temp
            strcat(buffer, temp);// it concat in the buffer
          }

        sprintf(temp, " sum:%d  ", dealer.sum);//store sum from the server in temp
        strcat(buffer, temp);// it concat in the buffer
        sendMessage(connection_fd, buffer, strlen(buffer));//send the dealer hand and sum to the client
        printf("\n%d %d  ", dealer.sum,hand.sum );

        if(hand.sum > dealer.sum)//if sum from the client is higher client win
        {
          sprintf(buffer, "WIN");
          sendMessage(connection_fd, buffer, strlen(buffer));
        /*  sprintf(buffer, "%d", hand.bet*2);
          sendMessage(connection_fd, buffer, strlen(buffer));*/
          finish = 1;
        }
        else if(hand.sum < dealer.sum)//if sum from the client is higher lose
        {
          if(dealer.sum > 21)// if sum from the dealer is higher than 21 you win
          {
            sprintf(buffer, "WIN");
            sendMessage(connection_fd, buffer, strlen(buffer));
            finish = 1;
          }
          else//otherwise you will lose
          {
            sprintf(buffer, "LOST");
            sendMessage(connection_fd, buffer, strlen(buffer));
            finish = 1;
          }
        }
        else
        {
          sprintf(buffer, "TIE");//if both hands are the same value
          sendMessage(connection_fd, buffer, strlen(buffer));
          finish = 1;
        }

      }
      else if(action == 3)//if the client exit
      {
        finish = 1;
      }

    }
    receiveMessage(connection_fd, buffer, BUFFER_SIZE);
    sprintf(buffer, "BYE");
    sendMessage(connection_fd, buffer, strlen(buffer));
}

int randomCard()//funtion for a random between  1 or 13
{
    int card =  1 + rand() % 13;
    if (card == 12|| card == 13)// if there is a queen or king the value will be a 10
        card = 10;
    return card;
}
