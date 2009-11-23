


void initAI();

void freeAI();



void setEnemyMove( unsigned char *inEnemyMove, int inEnemyLength );


void stepAI();


// returns NULL if move not ready yet
unsigned char *getAIMove( int *outMoveLength );


/*


FIXME:  what happens when we only need move from one player?
( like in case of moving inspector )

usage pattern:

while( enemy move not ready )
   stepAI

setEnemyMove

while( getAIMove == NULL )
   stepAI

*/
