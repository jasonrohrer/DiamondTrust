
#include "common.h"



// first 3 are player, second 3 are enemy, last 1 is UN inspector
#define numUnits 7
#define numPlayerUnits 3

void initUnits();

// return them to game start state in terms of salaries and bribes
void resetUnits();


void freeUnits();

// draws them on map
void drawUnits();

void stepUnits();


// draw a unit onto the current screen at the given (center of foot)
// position
void drawUnit( int inUnit, int inX, int inY );

// draw a total bribe marker onto the current screen at give (center of marker)
// position
void drawUnitBribe( int inUnit, int inX, int inY );



void setUnitSelectable( int inUnit, char inSelectable );

void setAllUnitsNotSelectable();

void setPlayerUnitsSelectable( char inSelectable );


// -1 to set none
void setActiveUnit( int inUnit );

int getActiveUnit();



// -1 if no unit hit
int getChosenUnit( int inClickX, int inClickY, char inOnlySelectable=true );


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

// true if we know that one of our units is bribed
char isAnyPlayerUnitKnownBribed();


// bribed unit in opponent's home region?
char isOpponentHomeBribed();

// true if we have a known-bribed unit at home
char isPlayerHomeKnownBribed();


// shows only moves that are known to player
void showUnitMoves( char inShow );

// shows all moves
void showAllUnitMoves( char inShow );



// getting tired of writing so many access functions...
// give access to the unit class directly
class Unit {
    public:
        Unit();
        
        int mRegion;
        int mDest;
        char mSelectable;
        int mSpriteID;
        int mArmSpriteID[5];
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
        
        char mEnemyContactSinceBribeHidden;
        
        int mLastBribingUnit;
        
        char mFlying;
        int mVehicleSpriteIndex;
        int mExecutionStep;
        int mNumExecutionSteps;

        int mNumDiamondsHeld;

        int mTripCost;

        int mAnimationFrameNumber;
        int mAnimationDirection;
        int mStepsUntilNextFrame;
        int mWavesLeftBeforeBreak;
        
    };


Unit *getUnit( int inUnit );



void executeUnitMoves();


// true if last execution done
char unitAnimationsDone();



// 0 for this player, 1 for opponent, -1 for neither
int getPlayerBribedInspector();

char isAnyUnitPayable();

char isAnyUnitBuyingDiamonds();

char isAnyUnitDepositingDiamonds();


char isAnyConfiscationNeeded();
