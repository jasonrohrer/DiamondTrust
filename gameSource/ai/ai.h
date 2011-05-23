


void initAI();

// back to starting state
void resetAI();


void freeAI();

// the number of candidate moves to test before picking best one
void setAINumMovesToTest( int inNumMoves );

int getAINumMovesToTest();


// if full speed is set to true, AI runs more simulations per step
// defaults to OFF
void toggleAICPUMode( char inFullSpeed );



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
