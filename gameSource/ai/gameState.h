

#include "common.h"



typedef struct intRange {
        int hi;
        int lo;
    } intRange;



typedef struct unit {
        intRange totalSalary;
        intRange totalBribe;
        
        int diamonds;

        int region;

        // -1 if unknown (or unset)
        int destination;

        intRange diamondBid;
        intRange inspectorBribe;

        // -1 if uknown
        // or [0..2] to indicate the opponent unit
        // that is bribing this unit
        // 3 if known and unbribed
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
    moveInspector,
    sellDiamonds 
    };



typdef struct gameState {
        
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
        

    } gameState;



void accumulateDiamonds( gameState *inState );



// transforms a state given two moves
// if preserve hidden info is true, then we ignore hidden effects of enemy
// move and build possibility spaces around them instead
gameState stateTransition( gameState inState, 
                           unsigned char *ourMove,
                           int ourLength,
                           unsigned char *enemyMove,
                           int enemyLength,
                           char inPreserveHiddenInfo );


// collapses a state into a single, randomly selected state within
// the possibility space of inState
gameState collapseState( gameState inState );


// takes a collapsed state as input
// returns 1 if we win, 0 for tie, -1 for enemy win
int playRandomGameUntilEnd( gameState inState );




