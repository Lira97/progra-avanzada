// The different types of operations available
typedef enum valid_operations {RECEIVEFILE,VERIFY, DOWNLOADFILE,REGISTER,GIVE, GETDIRECTORIES,CREATEDIRECTORY,CHECKA,DELETE, EXIT} operation_t;

// The types of responses available
typedef enum valid_responses {OK, INSUFFICIENT,CORRECT,INCORRECT,WRITTEN,ASK_ACCOUNT,FOUND,NOFOUND, BYE, ERROR,ASK, MENU,REGISTERS} response_t;
