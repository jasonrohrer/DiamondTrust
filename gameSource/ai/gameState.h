

#include "common.h"



typedef struct intRange {
        int hi;
        int lo;
        int t;  // true value
    } intRange;


inline intRange makeRange( int inHi, int inLo, int inT ) {
    intRange returnValue = { inHi, inLo, inT };
    return returnValue;
    }

inline intRange makeRange( int inHi ) {
    intRange returnValue = { inHi, inHi, inHi };
    return returnValue;
    }

inline void addToRange( intRange *inRange, int inValue ) {
    inRange->hi += inValue;
    inRange->lo += inValue;
    inRange->t += inValue;
    }




typedef struct unit {
        intRange totalSalary;
        intRange totalBribe;
        
        int diamonds;

        int region;

        // -1 if unknown (or unset)
        int destination;

        //intRange diamondBid;
        //intRange inspectorBribe;
        int diamondBid;
        int inspectorBribe;

        // -1 if none
        // or [0..2] to indicate the opponent unit
        int opponentBribingUnit;
        
    } unit;




/*
extern GameState *connectState;
extern GameState *accumulateDiamondsState;
extern GameState *salaryBribeState;
extern GameState *moveUnitsState;
extern GameState *depositDiamondsState;
extern GameState *confiscateState;
extern GameState *moveInspectorState;
extern GameState *buyDiamondsState;
extern GameState *sellDiamondsState;
extern GameState *flyHomeState;
extern GameState *gameEndState;
*/

/*
salaryBribeState.cpp:            sendMessage( message, (unsigned int)messageLength );
moveUnitsState.cpp:    sendMessage( message, (unsigned int)messageLength );
moveInspectorState.cpp:    sendMessage( message, (unsigned int)messageLength );
sellDiamondsState.cpp: 
*/


enum NextMove { 
    salaryBribe,
    moveUnits,
    moveUnitsCommit,
    moveInspector,
    sellDiamonds,
    sellDiamondsCommit
    };



typedef struct gameState {
        
        int monthsLeft;
        
        NextMove nextMove;



        intRange ourMoney;
        int knownOurTotalMoneyReceived;
        // money spent publicly (NOT including salaries and bribes)
        int knownOurTotalMoneySpent;
        
        int ourDiamonds;
        
        
        int ourDiamondsToSell;



        intRange enemyMoney;
        int knownEnemyTotalMoneyReceived;
        // money spent publicly (NOT including salaries and bribes)
        int knownEnemyTotalMoneySpent;
        
        int enemyDiamonds;


        int enemyDiamondsToSell;
        


        // 0 for player, 1 for enemy, 3 units each
        unit agentUnits[2][3];
        
        int inspectorRegion;

        // -1 if unknown
        // int inspectorDestination;


        // first two always have 0
        int regionDiamondCounts[8];
        

    } gameState;



void accumulateDiamonds( gameState *inState );



// -1 for no one, 0 for us, 1 for opponent
int whoMovesInspector( gameState *inState );




// transforms a state given two moves
// if preserve hidden info is true, then we ignore hidden effects of enemy
// move and build possibility spaces around them instead
gameState stateTransition( gameState *inState, 
                           unsigned char *ourMove,
                           int ourLength,
                           unsigned char *enemyMove,
                           int enemyLength,
                           char inPreserveHiddenInfo );


// collapses a state into a single, randomly selected state within
// the possibility space of inState
gameState collapseState( gameState *inState );


// takes a collapsed state as input
// returns positive number if we win (number of points in lead), 
//         0 for tie, 
//         negative number for enemy win (number of poitns trailing
int playRandomGameUntilEnd( gameState inState );



typedef struct possibleMove {
        unsigned char moveChars[12];

        // num chars used varies depending on game phase
        int numCharsUsed;

        // flag used for marking moves during testing
        int flag;
    } possibleMove;


// gets a randomly-selected possible move for player
// (use getMirrorState to extract move for enemy)
// force fresh pick during a commit state
possibleMove getPossibleMove( gameState *inState,
                              char inForceFreshPick = false );


possibleMove mutateMove( gameState *inState, possibleMove inMove,
                         int inMaxMutations );



// returns -1 if total number of possible moves is not practical to count
//   (in move state an salary/bribe state)
// returns number otherwise
//   (in move inspector state and sell state)
int getNumTotalPossibleMoves( gameState *inState );


// only call this if getNumTotalPossibleMoves did not return -1
// out moves must point to a possibleMove array with enough space for
//  getNumTotalPossibleMoves elements
void getAllPossibleMoves( gameState *inState, possibleMove *outMoves );



// returns a mirror of the state, swapping player for enemy
// only works on collapsed states
gameState getMirrorState( gameState *inState );

