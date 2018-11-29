/*
  Enrique Lira Martinez A01023351 ,Emiliano Abascal Gurria A01023234, Cesar Armando Valladares A01023503
  Client program to access nas
  This program connects to the server using sockets

  Gilberto Echeverria
  gilecheverria@yahoo.com
  29/03/2018
*/
#include "serverFunctions.h"

void closenas(nas_t * nas_data, locks_t * data_locks)
{
    printf("DEBUG: Clearing the memory for the thread\n");
    free(nas_data->account_array);
    free(data_locks->account_mutex);
}

char * Encrypt_password(char s[])	// this is a cesar encryption
{
 	char ch;
  int i;
  int key = 4;//chose a key to move the strings
      for(i = 0; s[i] != '\0'; ++i)
      {
          ch = s[i];

          if(ch >= 'a' && ch <= 'z'){
              ch = ch + key;

              if(ch > 'z'){
                  ch = ch - 'z' + 'a' - 1;
              }

              s[i] = ch;
          }
          else if(ch >= 'A' && ch <= 'Z'){
              ch = ch + key;

              if(ch > 'Z'){
                  ch = ch - 'Z' + 'A' - 1;
              }

              s[i] = ch;
          }
      }
      return(s);
}
int checkValidAccount(thread_data_t* data ,int account_num,char password[])
{
  account_t* account = &(data->nas_data->account_array[account_num]);
  pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
  pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  char result[255];
  char result2[255];
  int count=0;

  pthread_mutex_lock(account_l);// Locks account first and transactions second
  pthread_mutex_lock(transaction);
  account->infile = fopen("passwords.txt", "r");//open the password file
  if (account->infile == NULL)
   return 0;

   while ((read = getline(&line, &len, account->infile)) != -1)//read until it finish
  {
   if (count == account_num)//find the accont in file rows
   {
       if (line[read-1] == '\n')
           line[read-1] = 0;
           strcpy(result2, line);// copy the string a variable
      }
        count++;
    }
  fclose(account->infile);//close file
  strcpy(result, Encrypt_password(password));// compare the hash with the hash in the file

  pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second
  pthread_mutex_unlock(account_l);
  return (account_num >= 0 && account_num < NUM_ACCOUNTS && strcmp(result,result2)==0);//return the condition
}

int SaveUser(thread_data_t* data ,char password[])
{
    pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);
    int account =0;

    pthread_mutex_lock(transaction);

      char * line = NULL;
      size_t len = 0;
      ssize_t read;
      FILE *fp;

      fp = fopen("passwords.txt", "rb+");//open the password file
      if (fp == NULL)
       return 0;

       while ((read = getline(&line, &len, fp)) != -1)//read until it finish
      {
        account++;
      }
    fprintf(fp,"%s\n",Encrypt_password(password));//write the hash in the file
    free(line);//free variable
    fclose(fp);//close file

    pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second

    return account;//return the number of the account generated
}
int AddPersmisses(thread_data_t* data ,int account_num,char *other_account,char * path)
{
  account_t* account = &(data->nas_data->account_array[account_num]);
  pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
  pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);

  char * line = NULL;
  char file[100];

  pthread_mutex_lock(account_l);// Locks account first and transactions second
  pthread_mutex_lock(transaction);

  sprintf(file, "ServerFiles/%d", account_num);//put the path in string
  strcat(file, "/account.txt");
  printf("%s\n",other_account );
  printf("%s\n", file);
  account->infile = fopen(file, "a+");//open the file at the last line

  if(account->infile == NULL)
      return 0;

  fprintf(account->infile,"%s\n",other_account);//write the account
  free(line);//free variable
  fclose(account->infile);//close file

  pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second
  pthread_mutex_unlock(account_l);
  return 0;
}
int DeletePermissions(thread_data_t* data ,int account_num,char other_account[])
{
  account_t* account = &(data->nas_data->account_array[account_num]);
  pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
  pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);

  char file[100];
  char tempFile[100];
  char str [10];


    pthread_mutex_lock(account_l);// Locks account first and transactions second
    pthread_mutex_lock(transaction);

  sprintf(file, "ServerFiles/%d", account_num);
  strcat(file, "/account.txt");
  account->infile = fopen(file, "r");//open file
  if(account->infile == NULL){
    return 0;
  }

  // Temporary file
  sprintf(tempFile, "ServerFiles/%d", account_num);//put paht for the temp file
  strcat(tempFile, "/temp.txt");

  FILE * temp = fopen(tempFile, "w");
  while(fgets(str, 10, account->infile) != NULL){//until it finish
      int cmp = 1;
      for (int i = 0; i < strlen(other_account); i++){

          if (other_account[i] == str[0]){  // they are the same
          }
          else {
              cmp = 0;//they are diferent
          }
      }
      if (cmp == 1){
          // delete the permission
      }
      else {
          fprintf(temp, "%s", str);// compy to new temp file
      }
  }
  fclose(account->infile);//close file
  fclose(temp);//close file

  remove(file);//delete file
  rename(tempFile, file);//change the name

  pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second
  pthread_mutex_unlock(account_l);
  return 0;

}
int checkPermissions(thread_data_t* data ,int account_num,char *other_account)
{
  account_t* account = &(data->nas_data->account_array[account_num]);
  pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
  pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);
  char *line;
  char *nameUser = (char*)calloc(255, sizeof(char));
  char file[100];
  int result=0;
  pthread_mutex_lock(account_l);// Locks account first and transactions second
  pthread_mutex_lock(transaction);

  sprintf(nameUser, "%d/", account_num);
  strcat(file, "ServerFiles/");
  strcat(file,other_account);
  strcat(file, "/account.txt");//get the complete path
  account->infile = fopen(file, "rb");//open file
  if(account->infile == NULL)
      return 0;

  if (account->infile)
  {
      while (( line = read_line_by_line(account->infile)))//read all lines
      {
          if ( strstr(line,nameUser) )//if it found the account
          {
            fclose(account->infile);//close the file
            result = account_num;//return the account
            break;//get out of loop
          }
          free(line);//free variable
      }
  }

  fclose(account->infile);//cles file

  pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second
  pthread_mutex_unlock(account_l);
  return result;//return account
}
void SaveFile(thread_data_t* data, int account_num,unsigned char * message,int numbytes, char * fileName, char * path){
    account_t* account = &(data->nas_data->account_array[account_num]);
    pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
    pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);

    pthread_mutex_lock(account_l);// Locks account first and transactions second
    pthread_mutex_lock(transaction);
    strcat(path, fileName);
    account->infile = fopen(path, "wb+");
    if(account->infile == NULL)
      return ;

    if(message == NULL)
      return ;

    /* copy all the text into the buffer */
    fwrite (message , sizeof(char),numbytes, account->infile);
    fclose(account->infile);

    /* confirm we have read the file by
    outputing it to the console */
    pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second
    pthread_mutex_unlock(account_l);

}

long readFile(thread_data_t* data, int account_num, char * fileName, char * path){
    long numbytes = 0;
    account_t* account = &(data->nas_data->account_array[account_num]);
    pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
    pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);
    pthread_mutex_lock(account_l);// Locks account first and transactions second
    pthread_mutex_lock(transaction);
    char * filePath = (char *)calloc(255, sizeof(char));
    strcpy(filePath, path);
    strcat(filePath, fileName);
    account->infile = fopen(filePath, "rb");
    /* Get the number of bytes */
    fseek(account->infile, 0L, SEEK_END);
    numbytes = ftell(account->infile);
    /* copy all the text into the buffer */
    fclose(account->infile);


    /* confirm we have read the file by
    outputing it to the console */
    pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second
    pthread_mutex_unlock(account_l);
    return numbytes;
}
int Delete(thread_data_t* data,int account_num,char *fileName,char * path)
{
//    account_t* account = &(data->nas_data->account_array[account_num]);
    pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
    pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);

    pthread_mutex_lock(account_l);// Locks account first and transactions second
    pthread_mutex_lock(transaction);

    char *nameUser = (char*)calloc(255, sizeof(char));
    sprintf(nameUser, "%d/", account_num);//get path

    char * file = (char *) malloc(1 + strlen(path)+ strlen(nameUser) +strlen(fileName) );
    strcat(file, path);
    strcat(file, nameUser);
    strcat(file, fileName);//insert complete path
    int status;
    status = remove(file);//return a 0 if it eas Successful
    free(nameUser);//free memory
    free(file);//free memory
    pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second
    pthread_mutex_unlock(account_l);
    return status;
}
int getTransactionsInThread(thread_data_t* data)
{
    return getTransactions(data->nas_data, &(data->data_locks->transactions_mutex));
}
/**
 * Gets Transactions
 */
int getTransactions(nas_t* nas_data, pthread_mutex_t* transaction)
{
    int value;
    pthread_mutex_lock(transaction);
    value = nas_data->total_transactions;
    pthread_mutex_unlock(transaction);
    return value;
}

char * read_line_by_line(FILE *fin)
{
    char *buffer;
    char *tmp;
    int read_chars = 0;
    int bufsize = INITIAL_ALLOC;
    char *line = malloc(bufsize);

    if ( !line ) {
        return NULL;
    }// not file found

    buffer = line;

    while ( fgets(buffer, bufsize - read_chars, fin) )//get al lines
    {
      printf("%s\n",line );
        read_chars = strlen(line);

        if ( line[read_chars - 1] == '\n' )//skip enter in file
        {
          line[read_chars - 1] = '\0';
          return line;//return line
        }

        else {
            bufsize = 2 * bufsize;//resize the string
            tmp = realloc(line, bufsize);
            if ( tmp ) {
                line = tmp;
                buffer = line + read_chars;
            }
            else {
                free(line);//fre variable
                return NULL;
            }
        }
    }
    return NULL;
}

bool serverDownloadFile(float numbytes, float bytesTransfered, float bytesReceived, unsigned char * packet, int connection_fd, char * fileName){
  long sz = 0;
  FILE *fp = fopen(fileName, "wb+");
  while(numbytes > bytesTransfered){
    bytesReceived = read(connection_fd, packet, 1024);
    sz++;
    printf("\rReceived: %ld Mb.",(sz/1024));
  	fflush(stdout);
    fwrite(packet, 1,bytesReceived,fp);
    bytesTransfered += bytesReceived;
  }
  if(fwrite(packet, 1,bytesReceived,fp)){
    free(packet);
    fclose(fp);
    return true;
  }else{
    return false;
  }
}

bool serverUploadFile(float numbytes, float bytesTransfered, unsigned char * packet, int connection_fd, char * filePath){
  FILE *infile = fopen(filePath, "rb+");
  while (bytesTransfered < numbytes){//While the bytes sent by the client are lower than the total number of bytes that the file is made of, then if success is set to 0, then iterate from 0 to the maximum size of each packet, and assign each byte to the corresponding index of the file array that the server is about to send, set the success var to one, and send to the client the packet, the success variable is used so that if the client is done receiving the packet, then it tells the server to send the next one, this is done because with some files the client would not be done receiving each packet when the server would send another one, then print the download process.
    int nread = fread(packet,1,1024,infile);
    bytesTransfered += nread;
    write(connection_fd, packet, nread);
    printf("\rSending %f bytes", bytesTransfered);
    fflush(stdout);
  }
  fclose(infile);
  if(bytesTransfered >= numbytes){
    return true;
  }else{
    return false;
  }
}
