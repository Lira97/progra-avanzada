/*
    Enrique Lira Martinez A01023351 ,Emiliano Abascal Gurria A01023234, Cesar Armando Valladares A01023503
    Client program to access nas
    This program connects to the server using sockets

    Gilberto Echeverria
    gilecheverria@yahoo.com
    29/03/2018
*/

#include "fatal_error.h"

void fatalError(const char * message)
{
    perror(message);
    exit(EXIT_FAILURE);
}
