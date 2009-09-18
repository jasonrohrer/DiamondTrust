#include "units.h"
#include "map.h"
#include "platform.h"

class Unit {
    public:
        Unit();
        
        int mRegion;
        int mDest;
        char mSelectable;
        int mSpriteID;
        
    };


Unit::Unit() 
        : mRegion( -1 ), mDest( -1 ), mSelectable( false ),
          mSpriteID( -1 ) {

    }


Unit gameUnit[ numUnits ];



void initUnits() {
    gameUnit[ 0 ].mRegion = 0;
    gameUnit[ 1 ].mRegion = 0;
    gameUnit[ 2 ].mRegion = 0;

    // enemy
    gameUnit[ 3 ].mRegion = 1;
    gameUnit[ 4 ].mRegion = 1;
    gameUnit[ 5 ].mRegion = 1;

    // inspector
    gameUnit[ 6 ].mRegion = getRandom( numMapRegions - 2 ) + 2;
    }

void freeUnits() {
    }



void drawUnits() {
    
    }

void stepUnits() {
    }


void setUnitSelectable( int inUnit, char inSelectable ) {
    }


void setAllUnitsNotSelectable() {
    for( int u=0; u<numUnits; u++ ) {
        setUnitSelectable( u, false );
        }
    }



void setPlayerUnitsSelectable( char inSelectable ) {
    
    setUnitSelectable( 0, inSelectable );
    setUnitSelectable( 1, inSelectable );
    setUnitSelectable( 2, inSelectable );
    }



int getChosenUnit( int inClickX, int inClickY ) {
    }


int getUnitRegion( int inUnit ) {
    }


void setUnitDestination( int inUnit, int inRegion ) {
    }


int getUnitDestination( int inUnit ) {

    }


void executeMove( int inUnit ) {
    }


char animationsDone() {
    }
