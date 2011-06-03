#include "ai/gameState.h"
#include "ai/ai.h"

#include "units.h"
#include "map.h"
#include "gameStats.h"
#include "platform.h"

#include <assert.h>


static gameState currentState;

static unsigned char *externalEnemyMove = NULL;
static unsigned int externalEnemyMoveLength;

// to skip past camera data
static int externalEnemyMoveCharsToSkip = 0;


static char moveDone;



// track best move so far
possibleMove bestMoveSoFar;
int bestMoveScore = -10000000;

// test a new candidate move
possibleMove nextCandidateMove;
int nextCandidateMoveScore;
int nextCandidateMoveSimulations;

int numMovesTested = 0;
int totalMoveSimulationsForThisChoice = 0;



// number of simulations to run for each candidate
int maxNumSimulationsPerMove = 200;

// number of candidates to test before chosing best move
int maxNumMovesToTest = 4;


// Should new candidates be mutations of best or fresh, random moves?

// out of 10
// thus a value of 5 means that 50% of fresh replacement moves are mutations
// testing showed 5 was fine
int mutationVsRandomMixRatio = 7;

// 1 was found to be the best through testing
unsigned int maxMutationsPerMove = 1;




//int maxSimulationsPerStepAI = 100;

// for testing
// int maxSimulationsPerStepAI = 1000;
// this worked during testing on the PC
//int maxSimulationsPerStepAI = 100;
// too long for the DSi

// now that we have sound thread, we can handle even less!
// Also, didn't realize it, but 4 AI sims/step was causing FPS slowdown when
// drawing more complicated screens
//int simulationsPerStepSlowMode = 4;
int simulationsPerStepSlowMode = 2;


int maxSimulationsPerStepAI = simulationsPerStepSlowMode;

// problem!  This starves audio thread
//int simulationsPerStepFastMode = 1000;
int simulationsPerStepFastMode = 6;

void toggleAICPUMode( char inFullSpeed ) {
    if( inFullSpeed ) {
        maxSimulationsPerStepAI = simulationsPerStepFastMode;
        }
    else {
        maxSimulationsPerStepAI = simulationsPerStepSlowMode;
        }
    }





// define this to enable AI testing code.
// some of it doesn't compile on DS
//#define AI_TESTING_ENABLED

char testingAI = false;


#ifdef AI_TESTING_ENABLED

int currentTestingRound = 0;
#define numTestingRoundsSpace 5
// 40 was always worse than 20
//int testingRoundBatchSizes[ numTestingRoundsSpace ] ={ 10, 20, 40, 80, 160 };
// 20 was almost always worse than 15, and 25 never worth it
//int testingRoundBatchSizes[ numTestingRoundsSpace ] = { 5, 10, 15, 20, 25 };
// 7 seems to win most times
//int testingRoundBatchSizes[ numTestingRoundsSpace ] = { 3, 5, 7, 10, 13 };

// max num moves to test
int testingRoundParameter[ numTestingRoundsSpace ] = { 4, 8, 12, 16, 32 };

int numRunsPerTestingRound = 40;

float runBestScoreSums[ numTestingRoundsSpace ] = { 0, 0, 0, 0, 0 };
int runsSoFarPerRound[ numTestingRoundsSpace ] = { 0, 0, 0, 0, 0 };

int numTestingRounds = 5;
int maxTestingSimulationsPerMove = 600;

#include <stdio.h>
#include <stdlib.h>

FILE *testDataFile;

#endif






char stateChecked = true;



static void clearNextMove() {
    // printOut( "Clearing move\n" );
    
    // start with all not frozen
    for( int p=0; p<2; p++ ) {
        for( int u=0; u<3; u++ ) {
            unit *thisUnit = &( currentState.agentUnits[p][u] );
            thisUnit->moveFrozen = false;
            }
        }
    currentState.ourDiamondsToSellFrozen = false;
    currentState.enemyDiamondsToSellFrozen = false;
    

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

                
    if( !anyOfOurAgentsPotentiallyBribed ) {
        // enemy can't peek
        // the move enemy already set is it
    
        if( currentState.nextMove == moveUnitsCommit ) {
            
            // which parts can we see?
            for( int u=0; u<3; u++ ) {
                if( currentState.agentUnits[1][u].totalSalary.t <
                    currentState.agentUnits[1][u].totalBribe.t ) {
                                
                    // we have successfully bribed this enemy agent

                    // freeze its part of the move
                                
                    currentState.agentUnits[1][u].moveFrozen = true;
                    }
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
                currentState.enemyDiamondsToSellFrozen = true;
                }
                        
            }
                    
        }




    


    int numTotalMoves = getNumTotalPossibleMoves( &currentState );
    

    
    bestMoveSoFar = getGoodMove( &currentState );
    bestMoveScore = -10000000;
    bestMoveSoFar.flag = 0;
    
    nextCandidateMove = getGoodMove( &currentState );
    nextCandidateMoveScore = 0;
    nextCandidateMoveSimulations = 0;
    nextCandidateMove.flag = 0;
    
    if( numTotalMoves == 1 ) {
        // don't even need to test
        printOut( "Only one possible move found for %s, not running sims.\n",
                  nextMoveNames[ currentState.nextMove ] );
        moveDone = true;
        }
    
    totalMoveSimulationsForThisChoice = 0;
    
    numMovesTested = 0;
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





//#include "minorGems/util/stringUtils.h"

//extern char *statusMessage;

void initAI() {

#ifdef SDK_TWL
    printOut( "Got to AI check for console type\n" );

    unsigned int consoleType = OS_GetConsoleType();

    unsigned int console = consoleType & OS_CONSOLE_MASK;

    if( console == OS_CONSOLE_NITRO ||
        console == OS_CONSOLE_ISDEBUGGER ) {
        // DS is half the speed of DSi... do this to avoid slowdown
        simulationsPerStepSlowMode = 2;
        maxSimulationsPerStepAI = simulationsPerStepSlowMode;
        printOut( "Console type is NITRO\n" );
        //statusMessage = "NITRO";
        }
    else {
        printOut( "Console type is NOT NITRO\n" );
        //statusMessage = "NOT NITRO";
        }
    /*
    statusMessage = autoSprintf( "t=0x%02lX, c=0x%02lX, n=0x%02lX", 
                                 consoleType, console, OS_CONSOLE_NITRO );
    */
#endif


    #ifdef AI_TESTING_ENABLED
    if( testingAI ) {
        testDataFile = fopen( "aiTest.out", "w" );
        }
    #endif


    resetAI();
    }


void resetAI() {
        
    currentState.monthsLeft = getMonthsLeft();
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

    if( externalEnemyMove != NULL ) {
        delete [] externalEnemyMove;
        
        externalEnemyMove = NULL;
        }
    
    moveDone = false;

    clearNextMove();
    }


void freeAI() {
    }




void setAINumMovesToTest( int inNumMoves ) {
    if( !testingAI ) {

        maxNumMovesToTest = inNumMoves;
        printOut( "Setting AI num moves to: %d\n", maxNumMovesToTest );
        }
    
    }

    

int getAINumMovesToTest() {

    return maxNumMovesToTest;
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

    // compose our move into a string
    *outMoveLength = (unsigned int)( bestMoveSoFar.numCharsUsed );

    unsigned char *ourMove = 
        new unsigned char[ *outMoveLength ];
    memcpy( ourMove, bestMoveSoFar.moveChars, *outMoveLength );
    
    

    
        
    printOut( "Before transition...." );
    printStateKnowledgeStats();
    
    // apply both moves to our game state, preserving hidden info
    currentState = stateTransition( &currentState, 
                                    ourMove,
                                    (int)( *outMoveLength ),
                                    externalEnemyMove,
                                    (int)externalEnemyMoveLength,
                                    true );
    printOut( "Before applyKnowledge...." );
    printStateKnowledgeStats();
    

    // apply knowledge that we gained from this move
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
        checkCurrentStateMatches();
        stateChecked = true;
        }


    if( !moveDone ) {
        
        int gamesThisStep = 0;
        
        while( !moveDone && gamesThisStep < maxSimulationsPerStepAI ) {
            gamesThisStep++;
            
            // simulate one game for our next candidate


            // stop as soon as we exhaust total simulations we're allowed
            // to do
            if( numMovesTested >= maxNumMovesToTest ) {
                
                if( ! ( testingAI && 
                        currentState.nextMove == moveUnits ) ) {
                    
                    moveDone = true;
                    }
                else {
                    #ifdef AI_TESTING_ENABLED
        
                    runsSoFarPerRound[ currentTestingRound ] ++;
                    
                    runBestScoreSums[ currentTestingRound ] +=
                        bestMoveScore / 
                        (float)maxNumSimulationsPerMove;
                    
                    if( runsSoFarPerRound[ currentTestingRound ] >=
                        numRunsPerTestingRound ) {
                        
                        currentTestingRound ++;
                        
                        if( currentTestingRound >= numTestingRounds ) {
                            
                            // totally done
                            fprintf( testDataFile, 
                                     "\n\n***** Test results for %d:\n",
                                     maxNumSimulationsPerMove  );
                            
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
                            
                            
                            // start over with more simulations per move
                            currentTestingRound = 0;

                            maxNumSimulationsPerMove += 600;
                            maxNumMovesToTest = 
                                testingRoundParameter[ currentTestingRound ];
                            
                            if( maxNumSimulationsPerMove > 
                                maxTestingSimulationsPerMove ) {
                                // really done!
                                
                                fclose( testDataFile );
                                exit( 0 );
                                }
                            }
                        
                        
                        //batchSizeBeforeReplaceWorstMoves =
                        //    testingRoundParameter[ currentTestingRound ];
                        //mutationPoolSize = 
                        //    testingRoundParameter[ currentTestingRound ];
                        //mutationVsRandomMixRatio = 
                        //    testingRoundParameter[ currentTestingRound ];
                        //fractionOfMovesDiscarded =
                        //    testingRoundParameter[ currentTestingRound ];
                        //maxMutationsPerMove =
                        //    testingRoundParameter[ currentTestingRound ];
                        //useGoodMovesOnly = 
                        //    testingRoundParameter[ currentTestingRound ];
                        
                        maxNumMovesToTest = 
                            testingRoundParameter[ currentTestingRound ];

                        printOut( "***** Testing parameter %d\n",
                               testingRoundParameter[ currentTestingRound ] );
                        
                        }
                    
                    if( !moveDone ) {
                        // start over for another run
                        clearNextMove();
                        }

                    #endif
                    }
                
                
                }
            else {


                // pick a possible starting state (collapsing hidden
                // information)
                gameState startState = collapseState( &currentState );
                
                // pick a possible co-move for opponent
                gameState mirror = getMirrorState( &startState );


                // first, generate a fresh move
                possibleMove enemyMove = getGoodMove( &mirror,
                                                      true );
                

                gameState nextState = 
                    stateTransition( &startState,
                                     nextCandidateMove.moveChars,
                                     nextCandidateMove.numCharsUsed,
                                     enemyMove.moveChars,
                                     enemyMove.numCharsUsed,
                                     false );
                int result = playRandomGameUntilEnd( nextState );
        
                // track total score
                nextCandidateMoveScore += result;

                nextCandidateMoveSimulations ++;
                totalMoveSimulationsForThisChoice ++;


                if( nextCandidateMoveSimulations >= 
                    maxNumSimulationsPerMove ) {
                    
                    // done testing this candidate
                    numMovesTested ++;

                    // how did it do?

                    if( nextCandidateMoveScore > bestMoveScore ) {
                        // new leader
                        
                        bestMoveScore = nextCandidateMoveScore;
                        bestMoveSoFar = nextCandidateMove;
                        }
                    

                    // generate a new candidate
                    nextCandidateMoveSimulations = 0;
                    nextCandidateMoveScore = 0;
                    

                    if( (int)getRandom( 11 ) > 
                        mutationVsRandomMixRatio ) {
                        // fresh move
                        nextCandidateMove = getGoodMove( &currentState );
                        }
                    else {
                        // mutation of best so far
                        nextCandidateMove = mutateMove( &currentState, 
                                                        bestMoveSoFar,
                                                        maxMutationsPerMove );
                        nextCandidateMove.flag = 1;
                        }
                    

                    
                    printOut( "\n\n**** Intermediary %d-sim point: ",
                              totalMoveSimulationsForThisChoice );
                    
                    

                    printOut( "Score: %d, Move: ", bestMoveScore );
        
                    for( int c=0; c<bestMoveSoFar.numCharsUsed; c++ ) {
                        printOut( "%d, ", 
                                  (int)(char)bestMoveSoFar.moveChars[c] );
                        }
                    
                    if( bestMoveSoFar.flag ) {
                        printOut( " (MUT)" );
                        }
                    
                    printOut( "\n" );
                    
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
                (unsigned int)( enemyMove.numCharsUsed ) );
        externalEnemyMoveLength = (unsigned int)( enemyMove.numCharsUsed );
        }

    char isMoveUnitsState = false;
    if( currentState.nextMove == moveUnits || 
        currentState.nextMove == moveUnitsCommit ) {
    
        isMoveUnitsState = true;
        }    

        


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

