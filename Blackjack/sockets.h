/*
    Library with common sockets functions

  Enrique Lira Martinez
  A01023351

*/

#ifndef SOCKETS_H
#define SOCKETS_H
#define HAND_SIZE 30
// Receive a message and validate the return values
int receiveMessage(int connection_fd, char * buffer, int buff_length);


// Send a message and validate
void sendMessage(int connection_fd, char * buffer, int buff_length);

int startServer(char * port);

typedef struct  hand_struct{//create a struc for the client and the server
    int num;
    int cards[HAND_SIZE];
    int bet;
    int sum;
} hand_t;


#endif  /* not SOCKETS_H */
