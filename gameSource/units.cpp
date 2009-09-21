#include "units.h"
#include "map.h"
#include "platform.h"
#include "common.h"
#include "tga.h"
#include "sprite.h"
#include "Font.h"

#include "minorGems/util/stringUtils.h"

class Unit {
    public:
        Unit();
        
        int mRegion;
        int mDest;
        char mSelectable;
        int mSpriteID;
        int mDotSpriteID;
        int mBid;
    };


Unit::Unit() 
        : mRegion( -1 ), mDest( -1 ), mSelectable( false ),
          mSpriteID( -1 ) {

    }


Unit gameUnit[ numUnits ];

static int activeUnit = -1;

static int activeUnitSprite;

static int bidSprite;
static int bidW, bidH;
extern Font *font8;



static rgbaColor playerColor = { 0, 255, 0, 255 };
static rgbaColor enemyColor = { 255, 0, 0, 255 };
static rgbaColor white = {255, 255, 255, 255 };
static rgbaColor black = {0, 0, 0, 255 };
static rgbaColor inspectColor = {84, 84, 255, 255 };




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

    int w,h;
    rgbaColor *moveDotRGBA = readTGAFile( "moveDot.tga",
                                       &w, &h );
    
    
    if( moveDotRGBA == NULL || w != unitSpriteW || h != imageH ) {
        printOut( "Reading unit move dot file failed.\n" );
        return;
        }

    


    // 1 pixel row between each sprite image
    unitSpriteH = ((imageH + 1) /  4 ) - 1;
    
    int spriteIDs[ 4 ];
    int moveDotSpriteIDs[ 4 ];
    
    int i;
    
    for( i=0; i<4; i++ ) {
        
        rgbaColor *subImage = 
            &( unitRGBA[ i * (unitSpriteH + 1) * unitSpriteW ] );
        
        applyCornerTransparency( subImage, unitSpriteW * unitSpriteH );

        spriteIDs[i] = 
            addSprite( subImage, unitSpriteW, unitSpriteH );
    

        subImage = 
            &( moveDotRGBA[ i * (unitSpriteH + 1) * unitSpriteW ] );
        
        applyCornerTransparency( subImage, unitSpriteW * unitSpriteH );

        moveDotSpriteIDs[i] = 
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
        
        gameUnit[ i ].mDotSpriteID = moveDotSpriteIDs[ i ];
        gameUnit[ i + 3 ].mDotSpriteID = moveDotSpriteIDs[ i ];
        }
    
    // inspector
    gameUnit[ 6 ].mRegion = getRandom( numMapRegions - 2 ) + 2;
    gameUnit[ 6 ].mSpriteID = spriteIDs[ 3 ];
    gameUnit[ 6 ].mDotSpriteID = moveDotSpriteIDs[ 3 ];


    // destinations -- nowhere
    // bids -- none
    for( i=0; i<numUnits; i++ ) {
        gameUnit[ i ].mDest = gameUnit[ i ].mRegion;
        gameUnit[ i ].mBid = 0;
        }
    

    activeUnitSprite = loadSprite( "activeUnitHalo.tga", true );
    if( activeUnitSprite == -1 ) {
        printOut( "Failed to load unit halo.\n" );
        }

    bidSprite = loadSprite( "bid.tga", &bidW, &bidH, true );
    if( bidSprite == -1 ) {
        printOut( "Failed to load bid marker.\n" );
        }
    }



void freeUnits() {
    }



void drawUnits() {

    // draw paths first, under units
    for( int i=0; i<numUnits; i++ ) {
        rgbaColor c;
        if( i < 3 ) {
            c = playerColor;
            }
        else if( i<6 ) {
            c = enemyColor;
            }
        else {
            // inspector sprite already contains color
            c = white;
            }

        // trans
        c.a = 160;
        

        if( gameUnit[i].mDest != gameUnit[i].mRegion ) {
            // move chosen
            
            // draw line
            intPair start = 
                getUnitPositionInRegion( gameUnit[i].mRegion, i );
            
            intPair end = 
                getUnitPositionInRegion( gameUnit[i].mDest, i );
            
            int distance = intDistance( start, end );
            
            int numDots = distance / 7;
            
            for( int d=0; d<numDots; d++ ) {
                int dotX = 
                    ( d * end.x + 
                      ( (numDots - 1) - d ) * start.x ) / (numDots - 1 );
                int dotY = 
                    ( d * end.y + 
                      ( (numDots - 1) - d ) * start.y ) / (numDots - 1 );
                
                dotX -= unitSpriteW / 2;
                dotY -= unitSpriteH / 2;
                    

                drawSprite( gameUnit[i].mDotSpriteID, dotX, dotY, c );
                }

            // new layer for each line drawn
            startNewSpriteLayer();
            
            }
        }



    // now draw units on top
    
    for( int i=0; i<numUnits; i++ ) {
        rgbaColor c;
        if( i < 3 ) {
            c = playerColor;
            }
        else if( i<6 ) {
            c = enemyColor;
            }
        else {
            // inspector sprite already contains color
            c = white;
            }
        
        intPair pos = getUnitPositionInRegion( gameUnit[i].mRegion, i );
        
        // center
        pos.x -= unitSpriteW / 2;
        pos.y -= unitSpriteH;
        
        drawSprite( gameUnit[i].mSpriteID, 
                    pos.x, 
                    pos.y, 
                    c, gameUnit[i].mSelectable );
        if( activeUnit == i ) {
            drawSprite( activeUnitSprite, 
                        pos.x, 
                        pos.y, 
                        c, gameUnit[i].mSelectable );
            }

        }


    // finally, draw any bids

    // first markers
    for( int i=0; i<numUnits; i++ ) {

        if( gameUnit[i].mDest != gameUnit[i].mRegion ) {
            

            intPair end = 
                getUnitPositionInRegion( gameUnit[i].mDest, i );

            drawSprite( bidSprite, end.x - bidW / 2, end.y - bidH / 2,
                        white );
             
            }
        }
    startNewSpriteLayer();
    // then amounts
    for( int i=0; i<numUnits; i++ ) {

        if( gameUnit[i].mDest != gameUnit[i].mRegion ) {
            

            intPair end = 
                getUnitPositionInRegion( gameUnit[i].mDest, i );
            
            char *bidString = autoSprintf( "$%d", gameUnit[i].mBid );

            font8->drawString( bidString, 
                               end.x, 
                               end.y - 4,
                               black, 
                               alignCenter );

            delete [] bidString;
            }
        }
    
    
    
    }


void stepUnits() {
    }


void setUnitSelectable( int inUnit, char inSelectable ) {
    gameUnit[ inUnit ].mSelectable = inSelectable;
    }


void setAllUnitsNotSelectable() {
    for( int u=0; u<numUnits; u++ ) {
        setUnitSelectable( u, false );
        }
    }



void setActiveUnit( int inUnit ) {
    activeUnit = inUnit;
    }



int getActiveUnit() {
    return activeUnit;
    }




void setPlayerUnitsSelectable( char inSelectable ) {
    
    setUnitSelectable( 0, inSelectable );
    setUnitSelectable( 1, inSelectable );
    setUnitSelectable( 2, inSelectable );
    }



int getChosenUnit( int inClickX, int inClickY ) {
    for( int i=0; i<numUnits; i++ ) {
        if( gameUnit[i].mSelectable ) {

            intPair pos = getUnitPositionInRegion( gameUnit[i].mRegion, i );

            if( inClickY < pos.y &&
                inClickY > pos.y - unitSpriteH &&
                inClickX > pos.x - unitSpriteW / 2 &&
                inClickX < pos.x + unitSpriteW / 2 ) {
                
                // hit
                return i;
                }
            }
        }
                    
    return -1;
    }


int getUnitRegion( int inUnit ) {
    return gameUnit[ inUnit ].mRegion;
    }


void setUnitDestination( int inUnit, int inRegion ) {
    gameUnit[ inUnit ].mDest = inRegion;
    }


int getUnitDestination( int inUnit ) {
    return gameUnit[ inUnit ].mDest;
    }


void setUnitBid( int inUnit, int inBid ) {
    gameUnit[ inUnit ].mBid = inBid;
    }


void executeMove( int inUnit ) {
    }


char animationsDone() {
    }
