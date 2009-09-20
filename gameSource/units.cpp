#include "units.h"
#include "map.h"
#include "platform.h"
#include "common.h"
#include "tga.h"

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


static rgbaColor playerColor = { 0, 255, 0, 255 };
static rgbaColor enemyColor = { 255, 0, 0, 255 };
static rgbaColor white = {255, 255, 255, 255 };




int unitSpriteW;
int unitSpriteH;


void initUnits() {

    int imageH;
    
    rgbaColor *unitRGBA = readTGAFile( "units_simple.tga",
                                       &unitSpriteW, &imageH );
    
    
    if( unitRGBA == NULL ) {
        printOut( "Reading unit sprite file failed.\n" );
        return;
        }

    // 1 pixel row between each sprite image
    unitSpriteH = ((imageH + 1) /  4 ) - 1;
    
    int spriteIDs[ 4 ];
    
    int i;
    
    for( i=0; i<4; i++ ) {
        
        rgbaColor *subImage = 
            &( unitRGBA[ i * (unitSpriteH + 1) * unitSpriteW ] );
        
        applyCornerTransparency( subImage, unitSpriteW * unitSpriteH );

        spriteIDs[i] = 
            addSprite( subImage, unitSpriteW, unitSpriteH );
        }
    

    // player
    gameUnit[ 0 ].mRegion = 0;
    gameUnit[ 1 ].mRegion = 0;
    gameUnit[ 2 ].mRegion = 0;

    // enemy
    gameUnit[ 3 ].mRegion = 1;
    gameUnit[ 4 ].mRegion = 1;
    gameUnit[ 5 ].mRegion = 1;

    for( i=0; i<3; i++ ) {
        gameUnit[ i ].mSpriteID = spriteIDs[ i ];
        gameUnit[ i + 3 ].mSpriteID = spriteIDs[ i ];
        }
    
    // inspector
    gameUnit[ 6 ].mRegion = getRandom( numMapRegions - 2 ) + 2;
    gameUnit[ 6 ].mSpriteID = spriteIDs[ 3 ];
    }



void freeUnits() {
    }



void drawUnits() {
    
    for( int i=0; i<numUnits; i++ ) {
        rgbaColor c;
        if( i < 3 ) {
            c = playerColor;
            }
        else if( i<6 ) {
            c = enemyColor;
            }
        else {
            c = white;
            }
        
            
        drawSprite( gameUnit[i].mSpriteID, 100 + 20 * i, 100, c );
        }
    
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
