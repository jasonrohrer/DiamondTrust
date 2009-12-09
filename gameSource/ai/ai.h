


void initAI();

void freeAI();


void setAIThinkingTime( int inSeconds );

int getAIThinkingTime();



void setEnemyMove( unsigned char *inEnemyMove, unsigned int inEnemyLength );


void stepAI();


// returns NULL if move not ready yet
unsigned char *getAIMove( unsigned int *outMoveLength );


/*



usage pattern:

while( enemy move not ready )
   stepAI

setEnemyMove

while( getAIMove == NULL )
   stepAI


   EXCEPTION:  When moving the inspector, usage pattern skips setEnemyMove
               or getAIMove, depending on who is moving the inspector

*/
