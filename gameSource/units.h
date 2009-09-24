
#include "common.h"



// first 3 are player, second 3 are enemy, last 1 is UN inspector
#define numUnits 7
#define numPlayerUnits 3

void initUnits();

void freeUnits();

// draws them on map
void drawUnits();

void stepUnits();


void setUnitSelectable( int inUnit, char inSelectable );

void setAllUnitsNotSelectable();

void setPlayerUnitsSelectable( char inSelectable );


// -1 to set none
void setActiveUnit( int inUnit );

int getActiveUnit();



// -1 if no unit hit
int getChosenUnit( int inClickX, int inClickY );


int getUnitRegion( int inUnit );


// draws "pending move" arrow for player units
// set back to current region to cancel move
void setUnitDestination( int inUnit, int inRegion );

// returns unit's current region if unit has no destination
int getUnitDestination( int inUnit );


void setUnitBid( int inUnit, int inBid );
int getUnitBid( int inUnit );

intPair getUnitBidPosition( int inUnit );


void setUnitInspectorBribe( int inUnit, int inBribe );
int getUnitInspectorBribe( int inUnit );

intPair getUnitInspectorBribePosition( int inUnit );

void showInspectorBribe( int inUnit, char inShow );


intPair getUnitSalaryPosition( int inUnit );
intPair getUnitBribePosition( int inUnit );


void setMovePeeking( char inPeeking );


char isAnyOpponentBribed();

void showUnitMoves( char inShow );



// getting tired of writing so many access functions...
// give access to the unit class directly
class Unit {
    public:
        Unit();
        
        int mRegion;
        int mDest;
        char mSelectable;
        int mSpriteID;
        int mDotSpriteID;
        int mBid;
        int mInspectorBribe;
        char mShowInspectorBribe;

        int mTotalSalary;
        int mLastSalaryPayment;
        char mShowSalaryPayment;
        
        int mTotalBribe;
        int mLastBribePayment;
        char mShowBribePayment;
        
        int mLastBribingUnit;

        int mExecutionStep;
    };


Unit *getUnit( int inUnit );



void executeUnitMoves();


// true if last execution done
char unitAnimationsDone();

