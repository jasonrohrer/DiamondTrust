
// interface that hides whether opponent is networked or local AI


// true for local AI
// if networked, can call this before a connection is established
// okay to call this multiple times for playing new games and switching
// opponent type.  Don't need to call freeOpponent each time
void initOpponent( char inLocalAI );

// turns off AI opponent if one is running
void resetOpponent();



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
