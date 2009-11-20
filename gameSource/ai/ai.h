


void initAI();

void freeAI();



void setEnemyMove( unsigned char *inEnemyMove, int inEnemyLength );


void stepAI();


// returns NULL if move not ready yet
unsigned char *getAIMove( int *outMoveLength );


/*

usage pattern:

while( enemy move not ready )
   stepAI

setEnemyMove

while( getAIMove == NULL )
   stepAI

*/
