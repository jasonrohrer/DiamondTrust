#include "ai/gameState.h"

#include "units.h"
#include "map.h"
#include "gameStats.h"
#include "platform.h"

#include <assert.h>


static gameState currentState;

static unsigned char *externalEnemyMove;
static unsigned int externalEnemyMoveLength;

static char moveDone;




// scores for all possible moves at the current step
#define numPossibleMoves 16
int moveScores[ numPossibleMoves ];
int moveVisits[ numPossibleMoves ];

possibleMove moves[ numPossibleMoves ];

int numStepsTaken = 0;
int maxNumSteps = 20000;

int maxGamesPerStep = 400;




char stateChecked = true;




void clearNextMove() {
    for( int m=0; m<numPossibleMoves; m++ ) {
        moveScores[m] = 0;
        moveVisits[m] = 0;
        moves[m] = getPossibleMove( &currentState );
        }
    numStepsTaken = 0;
    }



static void checkEqual( int inA, int inB ) {
    assert( inA == inB );
    
    }



static void checkCurrentStateMatches() {
    printOut( "AI checking internal state against true game state...\n" );
    

    checkEqual( currentState.monthsLeft, getMonthsLeft() );
    
    // NextMove nextMove;

    
    // everything reversed (player is enemy in our local state)

    checkEqual( currentState.ourMoney.t, getPlayerMoney( 1 ) );
    
    //int knownOurTotalMoneyReceived;
    //int knownOurTotalMoneySpent;
        
    checkEqual( currentState.ourDiamonds, getPlayerDiamonds( 1 ) );


    checkEqual( currentState.enemyMoney.t, getPlayerMoney( 0 ) );
    
    //int knownEnemyTotalMoneyReceived;
    //int knownEnemyTotalMoneySpent;
        
    checkEqual( currentState.enemyDiamonds, getPlayerDiamonds( 0 ) );

    
        // 0 for player, 1 for enemy, 3 units each
    for( int p=0; p<2; p++ ) {
        for( int u=0; u<3; u++ ) {
            

            unit thisUnit =
                currentState.agentUnits[p][u];

            // swapped, enemy is us
            int swappedP = (p + 1) % 2;
            
            Unit *matchingUnit = getUnit( swappedP * numPlayerUnits + u );
            

            checkEqual( thisUnit.totalSalary.t, matchingUnit->mTotalSalary );
            checkEqual( thisUnit.totalBribe.t, matchingUnit->mTotalBribe );
            checkEqual( thisUnit.diamonds, matchingUnit->mNumDiamondsHeld );
            
            int swappedRegion = thisUnit.region;
            if( swappedRegion == 0 ) {
                swappedRegion = 1;
                }
            else if( swappedRegion == 1 ) {
                swappedRegion = 0;
                }
            
            checkEqual( swappedRegion, matchingUnit->mRegion );

            if( thisUnit.opponentBribingUnit == -1 ) {
                checkEqual( thisUnit.opponentBribingUnit, 
                            matchingUnit->mLastBribingUnit );
                }
            else {
                if( p == 0 ) {    
                    checkEqual( thisUnit.opponentBribingUnit, 
                                matchingUnit->mLastBribingUnit );
                    }
                else {
                    checkEqual( thisUnit.opponentBribingUnit + numPlayerUnits, 
                                matchingUnit->mLastBribingUnit );
                    }
                
                }
            }
        }
    
        
    checkEqual( currentState.inspectorRegion, getUnitRegion( numUnits - 1 ) );

    

    for( int r=0; r<8; r++ ) {
        checkEqual( currentState.regionDiamondCounts[r], 
                    getDiamondsInRegion(r) );
        }
    
    printOut( "...check passed\n" );
        
    }






void initAI() {
    currentState.monthsLeft = 8;
    currentState.nextMove = salaryBribe;

    currentState.ourMoney = makeRange( 18 );
    currentState.knownOurTotalMoneyReceived = 18;
    currentState.knownOurTotalMoneySpent = 0;
    currentState.ourDiamonds = 2;
    
    currentState.enemyMoney = makeRange( 18 );
    currentState.knownEnemyTotalMoneyReceived = 18;
    currentState.knownEnemyTotalMoneySpent = 0;
    currentState.enemyDiamonds = 2; 
    
    for( int p=0; p<2; p++ ) {
        for( int u=0; u<3; u++ ) {
            unit *thisUnit = &( currentState.agentUnits[p][u] );
            
            intRange zeroRange = { 0, 0 };
            

            thisUnit->totalSalary = zeroRange;
            thisUnit->totalBribe = zeroRange;
            thisUnit->diamonds = 0;
            thisUnit->region = p;
            //thisUnit->destination = p;
            thisUnit->diamondBid = 0;
            thisUnit->inspectorBribe = 0;
            thisUnit->opponentBribingUnit = -1;
            }
        }
    
    currentState.inspectorRegion = getUnitRegion( numUnits - 1 );
    //currentState.inspectorDestination = currentState.inspectorRegion;
    for( int r=0; r<8; r++ ) {
        currentState.regionDiamondCounts[r] = 0;
        }

    // accumulate to prepare for first move
    accumulateDiamonds( &currentState );

    externalEnemyMove = NULL;

    moveDone = false;

    clearNextMove();
    }


void freeAI() {
    }








// assumes that externalEnemyMove has been set
static unsigned char *pickAndApplyMove( unsigned int *outMoveLength ) {
    
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
    currentState = stateTransition( &currentState, 
                                    ourMove,
                                    *outMoveLength,
                                    externalEnemyMove,
                                    externalEnemyMoveLength,
                                    true );

    delete [] externalEnemyMove;
    externalEnemyMove = NULL;
    moveDone = false;
    

    clearNextMove();
    
    stateChecked = false;
    

    return ourMove;
    }




void setEnemyMove( unsigned char *inEnemyMove, unsigned int inEnemyLength ) {

    if( externalEnemyMove != NULL ) {
        // existing, unused move message waiting
        delete [] externalEnemyMove;
        externalEnemyMove = NULL;
        

        // NOTE:  this will happen when camera messages are passed as part
        // of the sellDiamondsState, and we'll simply discard them        
        }
    

    externalEnemyMove = new unsigned char[ inEnemyLength ];
    memcpy( externalEnemyMove, inEnemyMove, inEnemyLength );
    externalEnemyMoveLength = inEnemyLength;


    // in case of moveInspector state, we may not be making a move ourself

    if( currentState.nextMove == moveInspector &&
        whoMovesInspector( &currentState ) == 1 ) {
        
        // we've got all we need for state transition

        // doesn't matter that our move selection is not done, because it
        // won't be used by state transition anyway (treated as a dummy move)
        unsigned int ourMoveLength;
        
        unsigned char *ourMove = pickAndApplyMove( &ourMoveLength );
        
        // discard our move, because it's not needed by opponent
        delete [] ourMove;
        }
    }








void stepAI() {
    // check state once per month
    if( !stateChecked && currentState.nextMove == moveUnits ) {
        // FIXME:  turn back on, testing so it's off
        //checkCurrentStateMatches();
        stateChecked = true;
        }
    

    if( !moveDone ) {
        
        int gamesThisStep = 0;
        
        while( !moveDone && gamesThisStep < maxGamesPerStep ) {
            gamesThisStep++;

            // simulate one game for one of our moves
        
            
            // pick moves by weighting them according to their win scores
            // so far
            
            int normMoveScores[ numPossibleMoves ];
            int minScore = 100000;
            for( int m=0; m<numPossibleMoves; m++ ) {
                if( moveScores[ m ] < minScore ) {
                    minScore = moveScores[m];
                    }
                }
            // subtract minScore from each score, and add 1 to put all scores
            // in [1...) range (at start, before any scores known, all 
            // normalized scores are 1
            // use squares of scores to amplify effect
            int totalNormScores = 0;
            for( int m=0; m<numPossibleMoves; m++ ) {
                normMoveScores[m] = ( moveScores[ m ] - minScore ) + 1;

                normMoveScores[m] *= normMoveScores[m];
                totalNormScores += normMoveScores[m];
                }
            
            
            int weightThreshold = getRandom( totalNormScores );
            

            int chosenMove = -1;
            
            int weightPassedSoFar = 0;
            while( weightPassedSoFar <= weightThreshold ) {
                chosenMove ++;
                weightPassedSoFar += normMoveScores[chosenMove];
                }
            
            // this picks a move m with probability of 
            // normMoveScores[m] / totalNormScores
            
            // at start, this is a uniform distribution, but as wins/losses
            // build up, probability of picking good moves for additional
            // exploration grows
            

            // FIXME:  back to original uniform dist for testing
            chosenMove = getRandom( numPossibleMoves );
        
            // pick a possible starting state (collapsing hidden
            // information)
            gameState startState = collapseState( &currentState );
        
            // pick a possible co-move for opponent
            gameState mirror = getMirrorState( &startState );
            possibleMove enemyMove = getPossibleMove( &mirror );
        
            gameState nextState = 
                stateTransition( &startState,
                                 moves[chosenMove].moveChars,
                                 moves[chosenMove].numCharsUsed,
                                 enemyMove.moveChars,
                                 enemyMove.numCharsUsed,
                                 false );
            int result = playRandomGameUntilEnd( nextState );
        
            // old, track total score
            moveScores[ chosenMove ] += result;

            /*
            // new:  track (wins - losses)
            if( result < 0 ) {
                result = -1;
                }
            if( result > 0 ) {
                result = 1;
                }
            moveScores[ chosenMove ] += result;
            */
            moveVisits[ chosenMove ] ++;

            numStepsTaken ++;
        
            if( numStepsTaken >= maxNumSteps ) {
                moveDone = true;
                }
            else if( numStepsTaken == maxNumSteps / 2 ) {
                
                // print out stats, then clear them out and start over

                // do we settle on same choice again?
                

                printOut( "Move visit counts at halfway:\n" );
    
                for( int m=0; m<numPossibleMoves; m++ ) {
                    printOut( "[%d : %d], ", m, moveVisits[ m ] );
                    moveVisits[ m ] = 0;
                    }
                printOut( "\n\n" );

                printOut( "Move score counts at halfway:\n" );
    
                for( int m=0; m<numPossibleMoves; m++ ) {
                    printOut( "[%d : %d], ", m, moveScores[ m ] );
                    moveScores[m] = 0;
                    }
                printOut( "\n\n" );

                

                }
            }
        }
    
    }



// returns NULL if move not ready yet
unsigned char *getAIMove( unsigned int *outMoveLength ) {
    if( !moveDone ) {
        return NULL;
        }


    if( externalEnemyMove == NULL ) {
        // they didn't pass a move in.
        
        // is this the moveInspector state?
        if( currentState.nextMove != moveInspector ) {
            printOut( "Error:  no opponent move passed to ai before "
                      "getAIMove, and not moveInspector state\n" );
            
            return NULL;
            }
        
        if( whoMovesInspector( &currentState ) != 0 ) {
            printOut( "Error:  no opponent move passed to ai before "
                      "getAIMove, and AI is not moving the inspector\n" );
            return NULL;
            }
        

        // otherwise, we don't need their move (we're moving the inspector)

        // SO...
        // pick a fake move for them

        // pick a possible starting state (collapsing hidden
        // information)
        gameState startState = collapseState( &currentState );
        
        // pick a possible co-move for opponent
        gameState mirror = getMirrorState( &startState );
        possibleMove enemyMove = getPossibleMove( &mirror );


        externalEnemyMove = new unsigned char[ enemyMove.numCharsUsed ];
        memcpy( externalEnemyMove, enemyMove.moveChars, 
                enemyMove.numCharsUsed );
        externalEnemyMoveLength = enemyMove.numCharsUsed;
        }

    char isMoveUnitsState = false;
    if( currentState.nextMove == moveUnits || 
        currentState.nextMove == moveUnitsCommit ) {
    
        isMoveUnitsState = true;
        }
    

    // make sure our current state matches actual game
    // checkCurrentStateMatches();
    
    printOut( "Move visit counts:\n" );
    
    for( int m=0; m<numPossibleMoves; m++ ) {
        printOut( "[%d : %d], ", m, moveVisits[ m ] );
        }
    printOut( "\n\n" );
    printOut( "Move score counts:\n" );
    
    for( int m=0; m<numPossibleMoves; m++ ) {
        printOut( "[%d : %d], ", m, moveScores[ m ] );
        }
    printOut( "\n\n" );
    

    unsigned char *ourMove = pickAndApplyMove( outMoveLength );
    
    if( isMoveUnitsState ) {
        // swap home regions before giving move to opponent
        for( int u=0; u<3; u++ ) {
            unsigned char oldDest = ourMove[ u * 3 ];
            unsigned char newDest = oldDest;
            if( oldDest == 0 ) {
                newDest = 1;
                }
            else if( oldDest == 1 ) {
                newDest = 0;
                }

            ourMove[ u * 3 ] = newDest;
            }
        
        }
    
    
    printOut( "AI's chosen move: " );
    for( unsigned int i=0; i<*outMoveLength; i++ ) {
        printOut( "%d, ", (int)(char)ourMove[i] );
        }
    printOut( "\n" );
    
    return ourMove;
    }

