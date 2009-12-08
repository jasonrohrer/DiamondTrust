#include "ai/gameState.h"

#include "units.h"
#include "map.h"
#include "gameStats.h"
#include "platform.h"

#include <assert.h>


static gameState currentState;

static unsigned char *externalEnemyMove;
static unsigned int externalEnemyMoveLength;

// to skip past camera data
static int externalEnemyMoveCharsToSkip = 0;


static char moveDone;




// scores for all possible moves at the current step
#define possibleMoveSpace 256
int numPossibleMoves = 32;
int numPossibleMovesFilled = numPossibleMoves;
int moveScores[ possibleMoveSpace ];
int moveSimulations[ possibleMoveSpace ];
int moveSimulationsInScoreBatch[ possibleMoveSpace ];

int nextMoveToTest = 0;

possibleMove moves[ possibleMoveSpace ];

int moveSortMap[ possibleMoveSpace ];



//int maxNumSimulationsPerMove = 3720;//600;
int maxNumSimulationsPerMove = 300;//1200;//600;

// testing has showed 7 to be the best here
int batchSizeBeforeReplaceWorstMoves = 7;

// testing showed 7 to be the best here
int mutationPoolSize = 7;

// out of 10
// thus a value of 5 means that 50% of fresh replacement moves are mutations
// testing showed 5 was fine
int mutationVsRandomMixRatio = 5;

// 1 was found to be the best through testing
int maxMutationsPerMove = 1;

// out of 10
// 7 means lower 70% are discarded

// testing showed 5 (50%) was fine
int fractionOfMovesDiscarded = 5;



int finalBatchSize = 200;

//int maxSimulationsPerStepAI = 100;

// for testing
// int maxSimulationsPerStepAI = 1000;
int maxSimulationsPerStepAI = 100;






char testingAI = false;


int currentTestingRound = 0;
#define numTestingRoundsSpace 5
// 40 was always worse than 20
//int testingRoundBatchSizes[ numTestingRoundsSpace ] ={ 10, 20, 40, 80, 160 };
// 20 was almost always worse than 15, and 25 never worth it
//int testingRoundBatchSizes[ numTestingRoundsSpace ] = { 5, 10, 15, 20, 25 };
// 7 seems to win most times
//int testingRoundBatchSizes[ numTestingRoundsSpace ] = { 3, 5, 7, 10, 13 };

// maxMutationsPerMove
int testingRoundParameter[ numTestingRoundsSpace ] = { 1, 2, 3, 10, 10 };

int numRunsPerTestingRound = 40;

float runBestScoreSums[ numTestingRoundsSpace ] = { 0, 0, 0, 0, 0 };
int runsSoFarPerRound[ numTestingRoundsSpace ] = { 0, 0, 0, 0, 0 };

int numTestingRounds = 3;
int maxTestingPossibleMoves = 64;
int maxTestingSimulationsPerMove = 600;

#include <stdio.h>
#include <stdlib.h>

FILE *testDataFile;




static void sortMovesByScore() {
    
    char beenPicked[ numPossibleMovesFilled ];

    int nextScoreToPick = -10000000;

    for( int i=0; i<numPossibleMovesFilled; i++ ) {
        beenPicked[i] = false;
        }
    for( int i=0; i<numPossibleMovesFilled; i++ ) {
        
        int indexToPick = -1;
        int scoreToPick = 100000000;
        for( int m=0; m<numPossibleMovesFilled; m++ ) {
            if( ! beenPicked[ m ] && 
                moveScores[m] >= nextScoreToPick && 
                moveScores[m] < scoreToPick ) {
                
                scoreToPick = moveScores[m];
                indexToPick = m;
                }
            }

        moveSortMap[i] = indexToPick;
        
        beenPicked[ indexToPick ] = true;
        
        nextScoreToPick = scoreToPick;
        }
    }



static void printSortedMoves() {
    printOut( "Moves sorted by score:\n" );
    
    sortMovesByScore();
    
    for( int i=0; i<numPossibleMovesFilled; i++ ) {
        int indexToPrint = moveSortMap[ i ];

        printOut( "[%d] Score: %d, Move: ", indexToPrint, 
                  moveScores[indexToPrint] );
        
        for( int c=0; c<moves[indexToPrint].numCharsUsed; c++ ) {
            printOut( "%d, ", (int)(char)moves[indexToPrint].moveChars[c] );
            }

        if( moves[ indexToPrint ].flag ) {
            printOut( " (MUT)" );
            }
                    
        printOut( "\n" );
        }
    
    }







char stateChecked = true;


char equal( possibleMove inA, possibleMove inB ) {
    for( int c=0; c<inA.numCharsUsed; c++ ) {
        if( inA.moveChars[c] != inB.moveChars[c] ) {
            return false;
            }
        }

    return true;
    }




void clearNextMove() {
    // printOut( "Clearing move\n" );
    
    int numTotalMoves = getNumTotalPossibleMoves( &currentState );
    
    if( numTotalMoves == -1 || numTotalMoves > numPossibleMoves ) {
        // unpractical number of total moves, or too many to fit in our array

        // default to a fixed number of randomly chosen moves
        numPossibleMovesFilled = numPossibleMoves;
        
        
        // but in some cases, there are only a small number of available moves

        // if we detect too many collisions when trying to add a new move,
        // we can assume that this is the case, and limit the possible
        // move array there


        for( int m=0; m<numPossibleMovesFilled; m++ ) {
            
            // pick a unique move
            char collision = true;
            
            // in some situations, picking a unique move is impossible
            // (like if we're out of money, or there are no payable units)
            int collisionCount = 0;
            // bail after 100 collisions in a row
            
            while( collision && collisionCount < 100 ) {
                
                moves[m] = getPossibleMove( &currentState );
                
            
                // make sure this is not a collision with one already picked
                collision = false;
                
                for( int mm=0; mm < m && !collision; mm++ ) {
                    if( equal( moves[mm], moves[m] ) ) {
                        collision = true;
                        collisionCount++;
                        }
                    }                    
                }
            if( collision ) {
                // we bailed w/out finding another unique move
                // assume that this is it
                numPossibleMovesFilled = m;
                
                printOut( "Only found %d unique moves\n", m );
                }
            }
        }
    else {
        // fixed number of possible moves that is small enough for our array
        // use it!
        numPossibleMovesFilled = numTotalMoves;

        getAllPossibleMoves( &currentState, moves );        
        }
    

    for( int m=0; m<numPossibleMovesFilled; m++ ) {
        moveScores[m] = 0;
        moveSimulations[m] = 0;
        moveSimulationsInScoreBatch[m] = 0;
        moves[m].flag = 0;
            
        /*
              if( currentState.nextMove == salaryBribe ) {
              printOut( "Possible SB move: " );
              for( int i=0; i<moves[m].numCharsUsed; i++ ) {
              printOut( "%d, ", (int)(char)moves[m].moveChars[i] );
              }
              printOut( "\n" );
              }
            */
        }
    
    nextMoveToTest = 0;
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
    if( testingAI ) {
        testDataFile = fopen( "aiTest.out", "w" );
        }
    
    currentState.monthsLeft = 8;
    currentState.nextMove = salaryBribe;

    currentState.ourMoney = makeRange( 18 );
    currentState.knownOurTotalMoneyReceived = 18;
    currentState.knownOurTotalMoneySpent = 0;
    currentState.ourDiamonds = 2;

    currentState.ourDiamondsToSell = 0;
    
    
    currentState.enemyMoney = makeRange( 18 );
    currentState.knownEnemyTotalMoneyReceived = 18;
    currentState.knownEnemyTotalMoneySpent = 0;
    currentState.enemyDiamonds = 2; 
    
    currentState.enemyDiamondsToSell = 0;


    for( int p=0; p<2; p++ ) {
        for( int u=0; u<3; u++ ) {
            unit *thisUnit = &( currentState.agentUnits[p][u] );
            
            intRange zeroRange = { 0, 0 };
            

            thisUnit->totalSalary = zeroRange;
            thisUnit->totalBribe = zeroRange;
            thisUnit->diamonds = 0;
            thisUnit->region = p;
            thisUnit->destination = p;
            thisUnit->diamondBid = 0;
            thisUnit->inspectorBribe = 0;
            thisUnit->moveFrozen = false;
            
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




static void printRange( intRange inRange ) {
    printOut( "(h=%d, l=%d, t=%d)", inRange.hi, inRange.lo, inRange.t );
    }


static void printStateKnowledgeStats() {
    printOut( "Next move: %s\n", nextMoveNames[ currentState.nextMove ] );
    
    for( int p=0; p<2; p++ ) {
        printOut( "Player %d\n", p );
        
        if( p==0 ) {
            printOut( "  Money: " );
            printRange( currentState.ourMoney );
            
            printOut( "\n  Known received: %d\n", 
                      currentState.knownOurTotalMoneyReceived );
            
            printOut( "  Known spent: %d\n", 
                      currentState.knownOurTotalMoneySpent );
            }
        else {
            printOut( "  Money: " );
            printRange( currentState.enemyMoney );
            
            printOut( "\n  Known received: %d\n", 
                      currentState.knownEnemyTotalMoneyReceived );
            
            printOut( "  Known spent: %d\n", 
                      currentState.knownEnemyTotalMoneySpent );
            }
        

        for( int u=0; u<3; u++ ) {
            printOut( "  Agent %d\n", u );
            
            printOut( "    Salary: " );
            printRange( currentState.agentUnits[p][u].totalSalary );
            
            printOut( "\n    Bribe: " );
            printRange( currentState.agentUnits[p][u].totalBribe );
            printOut( "\n" );
            }
        }
    printOut( "\n" );
    }





// assumes that externalEnemyMove has been set
static unsigned char *pickAndApplyMove( unsigned int *outMoveLength ) {
    
    // pick move with highest score
    int highScore = -10000000;
    int chosenMove = -1;
    for( int m=0; m<numPossibleMovesFilled; m++ ) {
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
    
    

    
        
    printOut( "Before transition...." );
    printStateKnowledgeStats();
    
    // apply both moves to our game state, preserving hidden info
    currentState = stateTransition( &currentState, 
                                    ourMove,
                                    *outMoveLength,
                                    externalEnemyMove,
                                    externalEnemyMoveLength,
                                    true );
    printOut( "Before applyKnowledge...." );
    printStateKnowledgeStats();
    

    // apply knowledge that we gained from this move
    // FIXME:  this doesn't seem to be working right.  Set break here
    // and check it.
    currentState = applyKnowledge( &currentState );
    
    
    printOut( "After applyKnowledge...." );
    printStateKnowledgeStats();


    delete [] externalEnemyMove;
    externalEnemyMove = NULL;
    moveDone = false;
    

    clearNextMove();
    
    stateChecked = false;
    

    return ourMove;
    }




void setEnemyMove( unsigned char *inEnemyMove, unsigned int inEnemyLength ) {

    if( externalEnemyMoveCharsToSkip > 0 ) {
        
        // skip this message
        // do nothing with it

        externalEnemyMoveCharsToSkip -= inEnemyLength;
        
        if( externalEnemyMoveCharsToSkip < 0 ) {
            printOut( 
                "Error:  received more camera data than expected by AI\n" );
            
            externalEnemyMoveCharsToSkip = 0;
            }
        
        return;
        }
    



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

    
    if( ( currentState.nextMove == sellDiamonds ||
          currentState.nextMove == sellDiamondsCommit ) &&
        externalEnemyMoveLength == 3 ) {
        
        if( externalEnemyMove[1] == 1 ) {
            // third byte is # of forthcoming 300-byte messages containing
            // camera data
            
            externalEnemyMoveCharsToSkip = 300 * externalEnemyMove[2];
            }
        }


    
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
        checkCurrentStateMatches();
        stateChecked = true;
        }


    if( !moveDone ) {
        
        int gamesThisStep = 0;
        
        while( !moveDone && gamesThisStep < maxSimulationsPerStepAI ) {
            gamesThisStep++;

            // simulate one game for one of our moves
        
            /*
            // pick moves by weighting them according to their win scores
            // so far
            
            int normMoveScores[ numPossibleMoves ];
            int minScore = 2147483647;
            for( int m=0; m<numPossibleMovesFilled; m++ ) {
                if( moveScores[ m ] < minScore ) {
                    minScore = moveScores[m];
                    }
                }
            // subtract minScore from each score, and add 1 to put all scores
            // in [1...) range (at start, before any scores known, all 
            // normalized scores are 1
            // use squares of scores to amplify effect
            int totalNormScores = 0;
            for( int m=0; m<numPossibleMovesFilled; m++ ) {
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
            */

            // FIXME:  back to original uniform dist for testing
            // chosenMove = getRandom( numPossibleMovesFilled );
        
            

            // new:  just walk through moves, 
            // running maxNumSimulationsPerMove games for each move

            int chosenMove = nextMoveToTest;
            

            // stop as soon as we wrap back around
            if( moveSimulations[ chosenMove ] + 1 > 
                maxNumSimulationsPerMove ) {
                
                if( ! ( testingAI && 
                        currentState.nextMove == moveUnitsCommit ) ) {
                    
                    moveDone = true;
                    }
                else {
                    /*
                      
                    int currentTestingRound = 0;
                    #define numTestingRounds 5
                    int testingRoundBatchSizes[ numTestingRounds ] = { 10, 20, 40, 80, 160 };
                    
                    int numRunsPerTestingRound = 10;
                    
                    int runBestScoreSums[ numTestingRounds ] = { 0, 0, 0, 0, 0 };
                     */
                    sortMovesByScore();
                    
                    runsSoFarPerRound[ currentTestingRound ] ++;
                    
                    int bestMoveIndex = 
                        moveSortMap[ numPossibleMovesFilled - 1 ];

                    runBestScoreSums[ currentTestingRound ] +=
                        moveScores[ bestMoveIndex ] / 
                        (float)moveSimulationsInScoreBatch[ bestMoveIndex ];
                    
                    if( runsSoFarPerRound[ currentTestingRound ] >=
                        numRunsPerTestingRound ) {
                        
                        currentTestingRound ++;
                        
                        if( currentTestingRound >= numTestingRounds ) {
                            
                            // totally done
                            fprintf( testDataFile, 
                                     "\n\n***** Test results for %d, %d:\n",
                                     numPossibleMoves,
                                     maxNumSimulationsPerMove );
                            
                            for( int r=0; r<numTestingRounds; r++ ) {
                                fprintf( testDataFile,
                                         "Parameter %d, ave score %f\n",
                                         testingRoundParameter[r],
                                         runBestScoreSums[r] / 
                                         numRunsPerTestingRound );

                                // clear so we can use these again
                                // for other values
                                runBestScoreSums[r] = 0;
                                runsSoFarPerRound[r] = 0;
                                }
                            fflush( testDataFile );
                            

                            //int maxTestingPossibleMoves = 256;
                            //int maxTestingSimulationsPerMove = 1200;
                            
                            
                            // start over with more simulations per move
                            currentTestingRound = 0;

                            maxNumSimulationsPerMove *= 2;

                            if( maxNumSimulationsPerMove > 
                                maxTestingSimulationsPerMove ) {
                                
                                // start over with more possible moves
                                maxNumSimulationsPerMove = 300;
                                
                                numPossibleMoves *= 2;
                                
                                if( numPossibleMoves > 
                                    maxTestingPossibleMoves ) {
                                    
                                    // really done!
                                    
                                    fclose( testDataFile );
                                    exit( 0 );
                                    }
                                
                                }
                            }
                        
                        
                        //batchSizeBeforeReplaceWorstMoves =
                        //    testingRoundBatchSizes[ currentTestingRound ];
                        //mutationPoolSize = 
                        //    testingRoundParameter[ currentTestingRound ];
                        //mutationVsRandomMixRatio = 
                        //    testingRoundParameter[ currentTestingRound ];
                        //fractionOfMovesDiscarded =
                        //    testingRoundParameter[ currentTestingRound ];
                        maxMutationsPerMove =
                            testingRoundParameter[ currentTestingRound ];

                        printOut( "***** Testing parameter %d\n",
                               testingRoundParameter[ currentTestingRound ] );
                        
                        }
                    
                    if( !moveDone ) {
                        // start over for another run
                        clearNextMove();
                        }                    
                    }
                
                }
            else {


                // pick a possible starting state (collapsing hidden
                // information)
                gameState startState = collapseState( &currentState );
                
                // pick a possible co-move for opponent
                gameState mirror = getMirrorState( &startState );

                // if we KNOW that none of our agents have been bribed,
                // then we KNOW that the enemy can't peek
                // However, we might not have seen any of the enemy move
                // it depends on which of their agents have been bribed

                char anyOfOurAgentsPotentiallyBribed = false;
                for( int u=0; u<3; u++ ) {
                    if( currentState.agentUnits[0][u].totalSalary.t <
                        currentState.agentUnits[0][u].totalBribe.hi ) {
                        
                        // bribe MIGHT be higher than true salary
                        anyOfOurAgentsPotentiallyBribed = true;
                        }
                    }
                
                    

                // first, generate a fresh move
                possibleMove enemyMove = getPossibleMove( &mirror,
                                                          true );
                
                if( !anyOfOurAgentsPotentiallyBribed ) {
                    // enemy can't peek
                    // the move enemy already set is it
    
                    if( currentState.nextMove == moveUnitsCommit ) {

                        // which parts can we see
                        char anyPartsFrozen = false;
                        for( int u=0; u<3; u++ ) {
                            if( currentState.agentUnits[1][u].totalSalary.t <
                                currentState.agentUnits[1][u].totalBribe.t ) {
                                
                                // we have successfully bribed this enemy agent

                                // freeze its part of the move
                                
                                mirror.agentUnits[0][u].moveFrozen = true;
                                anyPartsFrozen = true;
                                }
                            }

                        if( anyPartsFrozen ) {    
                            // now with those parts frozen, re-gen a fresh move
                            // this will fill in guesses for the part of
                            // the move that we can't see
                            enemyMove = getPossibleMove( &mirror, true );
                            }
                        }
                    else if( currentState.nextMove == sellDiamondsCommit ) {
                        // do we have a bribed agent in the enemy's home?
                        
                        char bribedAgentInEnemyHome = false;
                        for( int u=0; u<3; u++ ) {
                            if( currentState.agentUnits[1][u].region == 1
                                &&
                                currentState.agentUnits[1][u].totalSalary.t <
                                currentState.agentUnits[1][u].totalBribe.t ) {
                                
                                bribedAgentInEnemyHome = true;
                                }
                            }

                        if( bribedAgentInEnemyHome ) {    
                            // replace move with one enemy already set
                            enemyMove = getPossibleMove( &mirror,
                                                         false );
                            }
                        
                        }
                    
                    }

                

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
                moveSimulations[ chosenMove ] ++;
                moveSimulationsInScoreBatch[ chosenMove ] ++;
        
                nextMoveToTest ++;
                nextMoveToTest %= numPossibleMovesFilled;
            

                // after batches of a given size, throw out the bottom
                // half of the moves, replace them with fresh random moves,
                // and reset all move scores

                // but save the last 1/4 of the simulations for deep
                // simulation of the final set of moves (to really tease
                // out the very best move)

                if( nextMoveToTest == 0 && 
                    ( moveSimulations[ chosenMove ] % 
                      batchSizeBeforeReplaceWorstMoves == 0
                      &&
                      moveSimulations[ chosenMove ] <
                      maxNumSimulationsPerMove - finalBatchSize ) ) {
                
                    

                    //printOut( "\n\n**** Intermediary %d-sim point: ",
                    //          moveSimulations[ chosenMove ] );
                    
                    //printSortedMoves();
                    sortMovesByScore();

                    int bestMoveIndex = 
                        moveSortMap[ numPossibleMovesFilled - 1 ];
                    /*
                    printOut( "High score (per sim, avg) so far = %f", 
                              moveScores[ bestMoveIndex ] / 
                       (float)moveSimulationsInScoreBatch[ bestMoveIndex ] );
                    
                    if( moves[ bestMoveIndex ].flag ) {
                        printOut( " (MUT)" );
                        }
                    printOut( "\n" );
                    */
                    
                    if( numPossibleMovesFilled == numPossibleMoves ) {
                        
                        //printOut( 
                        //    "Replacing bottom half of moves with fresh\n" );
                    
                        int numToReplace =
                            ( fractionOfMovesDiscarded * 
                              numPossibleMovesFilled ) / 10;


                        for( int i=0; i<numToReplace; i++ ) {
                            
                            // pick a unique move (not same as already
                            // picked or as remainder of top half)

                            char collision = true;
                            
                            while( collision ) {
                                
                                if( (int)getRandom( 11 ) > 
                                    mutationVsRandomMixRatio ) {
                                    // fresh move
                                    moves[ moveSortMap[i] ] =
                                        getPossibleMove( &currentState );
                                    moves[ moveSortMap[i] ].flag = 0;
                                    }
                                else {
                                    /*
                                    // mutation of one of the top 1/4 moves
                                    
                                    int moveToMutate = 
                                    getRandom( numPossibleMovesFilled / 4 )
                                    + (3 * numPossibleMovesFilled) / 4;
                                    */
                                    // mutate one of top N
                                    int moveToMutate = 
                                        ( numPossibleMovesFilled - 1 )
                                        - getRandom( mutationPoolSize );
                                    
                                    int ii = moveSortMap[ moveToMutate ];
                                    
                                    moves[ moveSortMap[i] ] =
                                        mutateMove( &currentState, 
                                                    moves[ ii ],
                                                    maxMutationsPerMove );
                                    moves[ moveSortMap[i] ].flag = 1;
                                    }

                                // make sure this is not a collision with one 
                                // already picked
                                collision = false;
                
                                for( int ii=0; ii < i && !collision; ii++ ) {
                                    if( equal( moves[ moveSortMap[ii] ], 
                                               moves[ moveSortMap[i] ] ) ) {
                                        collision = true;
                                        }
                                    } 
                                
                                if( !collision ) {
                                    // make sure no collision with top, 
                                    // retained moves
                                    
                                    for( int ii=numToReplace; 
                                         ii < numPossibleMovesFilled && 
                                             !collision; ii++ ) {

                                        if( equal( moves[ moveSortMap[ii] ],
                                              moves[ moveSortMap[i] ] ) ) {
                                            collision = true;
                                            }
                                        } 
                                    }
                                }                            
                            
                            }
                        
                        // reset all scores and batch counts
                        for( int i=0; i<numPossibleMovesFilled; i++ ) {
                            moveScores[i] = 0;
                            moveSimulationsInScoreBatch[i] = 0;
                            }
                        }
                    

                    

                    
                    // print out stats, then clear them out and start over
                    
                    // do we settle on same choice again?
                    
                    /*
                    printOut( "Move visit counts at halfway:\n" );
    
                    for( int m=0; m<numPossibleMovesFilled; m++ ) {
                        printOut( "[%d : %d], ", m, moveSimulations[ m ] );
                        moveSimulations[ m ] = 0;
                        }
                    printOut( "\n\n" );

                    printOut( "Move score counts at halfway:\n" );
    
                    for( int m=0; m<numPossibleMovesFilled; m++ ) {
                        printOut( "[%d : %d], ", m, moveScores[ m ] );
                        moveScores[m] = 0;
                        }
                    printOut( "\n\n" );
                    */
                

                    }
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
    
    printOut( "Move simulation counts:\n" );
    
    for( int m=0; m<numPossibleMovesFilled; m++ ) {
        printOut( "[%d : %d], ", m, moveSimulations[ m ] );
        }
    printOut( "\n\n" );
    printOut( "Move score counts:\n" );
    
    for( int m=0; m<numPossibleMovesFilled; m++ ) {
        printOut( "[%d : %d], ", m, moveScores[ m ] );
        }
    printOut( "\n\n" );

    printSortedMoves();
    

        


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

