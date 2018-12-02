/*
	Enrique Lira Martinez
	A01023351
	Matrix multiplication
  How to use:

      To run this program
      you should compile like this : "gcc matrix_multiplication.c main.c -o matrix_multiplication"
      and exceute like this: "./program Matrix1.txt Matrix2.txt"

      then the program will create a file called result.txt that cointains the result matrix

*/
#include "matrix_multiplication.h"//include the .h file with all the functions

int main(int argc, char ** argv)
{
  Multiply(readFile(argv[1]),readFile(argv[2]));//this takes 2 argument from the user and read it and then enter it will mutiply
}
