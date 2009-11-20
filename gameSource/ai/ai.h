


void initAI();

void freeAI();



void setEnemyMove( unsigned char *inEnemyMove, int inEnemyLength );


void stepAI();


// returns NULL if move not ready yet
unsigned char *getAIMove( int *outMoveLength );


/*

usage pattern:

setEnemyMove

while( getAIMove == NULL )
   stepAI

*/
