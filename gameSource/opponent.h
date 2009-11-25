
// interface that hides whether opponent is networked or local AI


// true for local AI
// if networked, can call this before a connection is established
void initOpponent( char inLocalAI );


void freeOpponent();


void stepOpponent();





// 1 if a connection has been established and is still up, 
// 0 if still connecting, 
// -1 on error
int checkOpponentConnectionStatus();


// inMessage destroyed by caller
void sendOpponentMessage( unsigned char *inMessage, unsigned int inLength );


// poll for new incoming message
// returns NULL if no message ready
// result freed by caller
unsigned char *getOpponentMessage( unsigned int *outLength );
