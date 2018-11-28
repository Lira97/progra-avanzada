
#include "serverFunctions.h"

void closeBank(bank_t * bank_data, locks_t * data_locks)
{
    printf("DEBUG: Clearing the memory for the thread\n");
    free(bank_data->account_array);
    free(data_locks->account_mutex);
}

/*
    Return true if the account provided is within the valid range,
    return false otherwise
*/
char * Encrypt_password(char s[])	// OR char return_and_pass(char *s)
{
 	char ch;
  int i;
  int key = 4;
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
int checkValidAccount(int account,char password[])
{
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  char result[255];
  char result2[255];
  FILE *fp;
  int count=0;
  fp = fopen("passwords.txt", "r");
  if (fp == NULL)
   return 0;

   while ((read = getline(&line, &len, fp)) != -1)
  {
   if (count == account)
   {
       if (line[read-1] == '\n')
           line[read-1] = 0;
           strcpy(result2, line);
      }
        count++;
    }
  fclose(fp);
  strcpy(result, Encrypt_password(password));
  return (account >= 0 && account < NUM_ACCOUNTS && strcmp(result,result2)==0);
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

      fp = fopen("passwords.txt", "rb+");
      if (fp == NULL)
       return 0;

       while ((read = getline(&line, &len, fp)) != -1)
      {
        account++;
      }
    fprintf(fp,"%s\n",Encrypt_password(password));
    free(line);
    fclose(fp);

    pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second

    return account;
}
int AddPersmisses(thread_data_t* data ,int account_num,char other_account[])
{
    account_t* account = &(data->bank_data->account_array[account_num]);
    pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
    pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);
  
    char * line = NULL;
    char file[100];

    pthread_mutex_lock(transaction);

    sprintf(file, "%d", account_num);
    strcat(file, "/account.txt");
    printf("%s\n",other_account );
    printf("%s\n", file);
    account->infile = fopen(file, "a+");

    if(account->infile == NULL)
        return 0;

    fprintf(account->infile,"%s\n",other_account);
    free(line);
    fclose(account->infile);

    pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second
    pthread_mutex_unlock(account_l);
    return 0;
}
void SaveFile(thread_data_t* data, int account_num,unsigned char * message,int numbytes, char * fileName, char * path, int flag){
    account_t* account = &(data->bank_data->account_array[account_num]);
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
void Delete(thread_data_t* data, int account_num)
{
//    account_t* account = &(data->bank_data->account_array[account_num]);
    pthread_mutex_t* account_l = &(data->data_locks->account_mutex[account_num]);
    pthread_mutex_t* transaction = &(data->data_locks->transactions_mutex);

    pthread_mutex_lock(account_l);// Locks account first and transactions second
    pthread_mutex_lock(transaction);
    int status;

    printf("Enter name of a file you wish to delete\n");

    status = remove("ServerFiles/output.mp4");

    if (status == 0)
      printf("file deleted successfully.\n");
    else
    {
      printf("Unable to delete the file\n");
      perror("Following error occurred");
    }

    pthread_mutex_unlock(transaction);  // Unlocks transactions first and account second
    pthread_mutex_unlock(account_l);


}
int getTransactionsInThread(thread_data_t* data)
{
    return getTransactions(data->bank_data, &(data->data_locks->transactions_mutex));
}
/**
 * Gets Transactions
 */
int getTransactions(bank_t* bank_data, pthread_mutex_t* transaction)
{
    int value;
    pthread_mutex_lock(transaction);
    value = bank_data->total_transactions;
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
    }

    buffer = line;

    while ( fgets(buffer, bufsize - read_chars, fin) )
    {
      printf("%s\n",line );
        read_chars = strlen(line);

        if ( line[read_chars - 1] == '\n' )
        {
          line[read_chars - 1] = '\0';
          return line;
        }

        else {
            bufsize = 2 * bufsize;
            tmp = realloc(line, bufsize);
            if ( tmp ) {
                line = tmp;
                buffer = line + read_chars;
            }
            else {
                free(line);
                return NULL;
            }
        }
    }
    return NULL;
}
