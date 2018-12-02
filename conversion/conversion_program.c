/*
	Enrique Lira Martinez
	A01023351
	Base conversions
*/

// C program to convert a number from any base
// to any base

/*to compile this program you mist need to enter this command in your terminal
'gcc conversion_program.c -o conversion_program -Wall'
and here is and example tu run the program
'./conversion_program -i 16 -o 2 ABC 5D 24'
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
In this program, two functions were created,
one is to move from any base to a decimal and the other converts any base to a decimal.
*/
void convertToDecimal(int input, int output,char* dest);
void revertDecimal(int number,int base);

int main(int argc, char ** argv)
{
	int option = 0;
	int input = 0;
	int output = 0;

	while((option = getopt(argc, argv, "i:o:")) != -1 )//the getopt is used to read the user's input and output
	{
		switch(option)//The switch is used to verify the input or output
		{
			case 'i':
				input = atoi(optarg);//the argument -i is read and converted into an integer to be used in the following functions
				break;
			case 'o':
				output = atoi(optarg);//the argument -o is read and converted into an integer to be used in the following functions
				break;
			default:
				break;
		}
	}
	if(argc < 1 || input < 2 || output < 2 || input > 32 || output > 32)//check if the bases are between 2 and 32 and check if the number is greater than you want to change is greater than 0
  {
		printf("Check your inputs");
	}
  else
  {
    for (int index = optind; index < argc; index++)//this loop goes through all the arguments that are after the input and output with the 'optind' option
    {
      printf("Converting %s from base %i to base %i\n\n",argv[index], input, output);
      convertToDecimal(input, output,argv[index]);//enters to the function that convert the argument to a decimal
    }
  }
	return 0;
}

void convertToDecimal(int input, int output,char* argument)
{
  int power = 1;//this variable is use to generate the power of each element
  int number = 0;//this variable stores the value that is generated when it's converting to decimal
  int realVal = 0;//this variable is used to know the value in integer of each element of the array of char

  for (int index = strlen(argument) - 1; index >= 0; index--)//this loop check the entire argument to define the number that it will use
  {
		//this 'if' check if the number is between 0 to 9 or A to Z and it rest the number from the ascii table to get the number
      if (argument[index] >= '0' && argument[index] <= '9')
          realVal = argument[index] - '0';//if the element is between 0 and 9, only the position of 0 in the ASCII table is subtracted
      else
          realVal = argument[index] - 'A' + 10;//if the element is a letter is the position of the letter A in the ASCII table to obtain the integer

      if (realVal >= input)//here we check if the number is greater than the input base
      {
           printf("Invalid Number");
           return;
      }
			// then it use the algorithm to turn everything into base 10
      number += realVal * power;
      power = power * input;
  }
	revertDecimal(number,output);//enter to the revert function to change between a decimal to any other base
}

void revertDecimal(int number,int base)
{
	char result[100];//it creates a array of chars for the final result but we must consider that it only can store 100 elements
	int index = 0;//it is an index for the next while loop
	while (number > 0)//this loop check the entire number to define the new base
	{
		//this if check if the number is between 0 to 9 or A to Z and it rest the number from the ascii table and store the letter int the array result
		if ((number % base) >= 0 && (number % base) <= 9)
			result[index++] = (number % base) + '0';//if the element is between 0 and 9, only the position of 0 in the ASCII table is subtracted but now in the form of char
    else
			result[index++] = (number % base) - 10 + 'A';//if the element is a letter is the position of the letter A in the ASCII table to obtain the char

			number /= base;//the number is divided by the bases to get the remening until it will be 0
	}
	result[index] = '\0';//it stop from reading

	int length = strlen(result);//here we check the length of the array for the next loop
	for (int i = 0; i < length/2; i++)// Reverse the result with a swap that goes from the begin to the middle of the array
	{
			char temporal = result[i];
			result[i] = result[length-i-1];
			result[length-i-1] = temporal;
	}

	printf("Converted to base %d:%s\n\n",base,result);//the result from the change of decimal to the desired base is printed
}
