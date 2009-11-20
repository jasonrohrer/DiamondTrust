#include "ai/gameState.h"

#include "units.h"
#include "platform.h"


static gameState currentState;

static unsigned char *enemyMove;
static int enemyMoveLength;

static char moveDone;




// scores for all possible moves at the current step
#define numPossibleMoves 100
int moveScores[ numPossibleMoves ];

possibleMove moves[ numPossibleMoves ];

int numStepsTaken = 0;
int maxNumSteps = 1000;











void clearNextMove() {
    for( int m=0; m<numPossibleMoves; m++ ) {
        moveScores[m] = 0;
        moves[m] = getPossibleMove( currentState );
        }
    numStepsTaken = 0;
    }


/*
int monthsLeft;
        
        NextMove nextMove;



        int ourMoney;
        int ourDiamonds;
        
        intRange enemyMoney;
        int enemyDiamonds;

        // 0 for player, 1 for enemy, 3 units each
        unit agentUnits[2][3];
        
        int inspectorRegion;

        // -1 if unknown
        int inspectorDestination;


        // first two always have 0
        int regionDiamondCounts[8];
*/
void initAI() {
    currentState.monthsLeft = 8;
    currentState.nextMove = salaryBribe;
    currentState.ourMoney = 18;
    currentState.ourDiamonds = 2;
    currentState.enemyMoney = { 18, 18 };
    currentState.enemyDiamonds = 2; 
    for( int p=0; p<2; p++ ) {
        for( int u=0; u<3; u++ ) {
            unit *thisUnit = &( currentState.agentUnits[p][u] );
            
            thisUnit->totalSalary = { 0, 0 };
            thisUnit->totalSalary = { 0, 0 };
            thisUnit->diamonds = 0;
            thisUnit->region = p;
            thisUnit->destination = p;
            thisUnit->diamondBid = { 0, 0 };
            thisUnit->inspectorBribe = { 0, 0 };
            thisUnit->opponentBribingUnit = 3;
            }
        }
    
    currentState.inspectorRegion = getUnitRegion( numUnits - 1 );
    currentState.inspectorDestination = currentState.inspectorRegion;
    for( int p=0; p<2; p++ ) {
        currentState.regionDiamondCounts[r] = 0;
        }

    // accumulate to prepare for first move
    accumulateDiamonds( &currentState );

    enemyMove = NULL;

    moveDone = false;

    clearNextMove();
    }


void freeAI() {
    }




void setEnemyMove( unsigned char *inEnemyMove, int inEnemyLength ) {
    enemyMove = new unsigned char[ inEnemyLength ];
    memcpy( enemyMove, inEnemyMove, inEnemyLength );

    moveDone = false;
    }








void stepAI() {
    if( !moveDone ) {
        
        // simulate one game for one of our moves
        
        int chosenMove = getRandom( 0, numPossibleMoves );
        
        // pick a possible starting state (collapsing hidden
        // information)
        gameState startState = collapseState( currentState );
        
        // pick a possible co-move for opponent
        gameState mirror = getMirrorState( startState );
        possibleMove enemyMove = getPossibleMove( mirror );
        
        gameState nextState = 
            stateTransition( startState,
                             moves[chosenMove].moveChars
                             moves[chosenMove].numCharsUsed,
                             enemyMove.moveChars,
                             enemyMove.numCharsUsed,
                             false );
        int result = playRandomGameUntilEnd( nextState );
        
        moveScores[ chosenMove ] += result;

        numStepsTaken ++;
        
        if( numStepsTaken >= maxNumSteps ) {
            moveDone = true;
            }
        }
    }



// returns NULL if move not ready yet
unsigned char *getAIMove( int *outMoveLength ) {
    if( !moveDone ) {
        return NULL;
        }
    
    
    // pick move with highest score
    int highScore = -10000;
    int chosenMove = -1;
    for( int m=0; m<numPossibleMoves; m++ ) {
        if( moveScores[m] > highScore ) {
            chosenMove = m;
            highScore = moveScores[m];
            }
        }
    

    // compose our move into a string
    *outMoveLength = moves[chosenMove].numCharsUsed;

    unsigned char *ourMove = 
        new unsigned char[ *outMoveLength ];
    memcpy( ourMove, moves[chosenMove].moveChars, *outMoveLength );
    


    // apply both moves to our game state, preserving hidden info
    currentState = stateTransition( currentState, 
                                    ourMove,
                                    *outMoveLength,
                                    enemyMove,
                                    enemyMoveLength,
                                    true );

    delete [] enemyMove;
    enemyMove = NULL;
    moveDone = false;
    

    clearNextMove();
    

    return ourMove;
    }

