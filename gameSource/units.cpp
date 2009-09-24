#include "units.h"
#include "map.h"
#include "platform.h"
#include "common.h"
#include "tga.h"
#include "sprite.h"
#include "Font.h"
#include "colors.h"


#include "minorGems/util/stringUtils.h"




Unit::Unit() 
        : mRegion( -1 ), mDest( -1 ), mSelectable( false ),
          mSpriteID( -1 ) {

    }


Unit gameUnit[ numUnits ];

static int activeUnit = -1;
static char peeking = false;
static char executing = true;
static char showMoves = false;
static char showAllMoves = false;

static int activeUnitSprite;

static int bidSprite;
static int bidW, bidH;
extern Font *font8;

static int bribedMarkerSprite;



//static rgbaColor inspectorColor = {84, 84, 255, 255 };




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
    gameUnit[ 0 ].mRegion = 2;
    gameUnit[ 1 ].mRegion = 0;
    gameUnit[ 2 ].mRegion = 0;

    // enemy
    gameUnit[ 3 ].mRegion = 2;
    gameUnit[ 4 ].mRegion = 1;
    gameUnit[ 5 ].mRegion = 1;

    for( i=0; i<3; i++ ) {
        gameUnit[ i ].mSpriteID = spriteIDs[ i ];
        gameUnit[ i + 3 ].mSpriteID = spriteIDs[ i ];
        
        gameUnit[ i ].mDotSpriteID = moveDotSpriteIDs[ i ];
        gameUnit[ i + 3 ].mDotSpriteID = moveDotSpriteIDs[ i ];


        gameUnit[ i ].mTotalSalary = 0;
        gameUnit[ i + 3].mTotalSalary = 0;
        gameUnit[ i ].mLastSalaryPayment = 0;
        gameUnit[ i + 3].mLastSalaryPayment = 0;


        gameUnit[ i ].mTotalBribe = 0;
        gameUnit[ i + 3].mTotalBribe = 0;
        gameUnit[ i ].mLastBribePayment = 0;
        gameUnit[ i + 3].mLastBribePayment = 0;
        gameUnit[ i ].mLastBribingUnit = -1;
        gameUnit[ i + 3].mLastBribingUnit = -1;
        }
    
    // inspector 
    // always starts in last region
    gameUnit[ 6 ].mRegion = numMapRegions - 1;
    gameUnit[ 6 ].mSpriteID = spriteIDs[ 3 ];
    gameUnit[ 6 ].mDotSpriteID = moveDotSpriteIDs[ 3 ];


    // destinations -- nowhere
    // bids -- none
    for( i=0; i<numUnits; i++ ) {
        gameUnit[ i ].mDest = gameUnit[ i ].mRegion;
        gameUnit[ i ].mBid = 0;
        gameUnit[ i ].mExecutionStep = 0;

        gameUnit[ i ].mNumDiamondsHeld = 0;
        }
    

    activeUnitSprite = loadSprite( "activeUnitHalo.tga", true );
    
    bidSprite = loadSprite( "bid.tga", &bidW, &bidH, true );
    
    bribedMarkerSprite = loadSprite( "bribedMarker.tga", true );

    }



void freeUnits() {
    }


Unit *getUnit( int inUnit ) {
    return &( gameUnit[ inUnit ] );
    }



char getUnitMoveVisible( int inUnit ) {
    if( inUnit < numPlayerUnits ) {
        return true;
        }
    else if( showAllMoves ) {
        return true;
        }
    else if( peeking ) {
        if( gameUnit[inUnit].mTotalSalary < gameUnit[inUnit].mTotalBribe ) {
            return true;
            }
        }
    
    return false;
    }



intPair getUnitCurrentPosition( int inUnit ) {
    int i=inUnit;
    
    intPair start = getUnitPositionInRegion( gameUnit[i].mRegion, i );
    intPair end = getUnitPositionInRegion( gameUnit[i].mDest, i );

    int progress = gameUnit[i].mExecutionStep;
    if( progress == 0 ) {
        return start;
        }

    int numSteps = gameUnit[i].mNumExecutionSteps;
    if( progress == numSteps ) {
        return end;
        }
    
    // else blend
    intPair result = end;
    result.x *= progress;
    result.y *= progress;
    
    result.x += (numSteps - progress) * start.x;
    result.y += (numSteps - progress) * start.y;
    
    result.x /= numSteps;
    result.y /= numSteps;
        
    return result;
    }


int getHomeRegion( int inUnit ) {
    if( inUnit < numPlayerUnits ) {
        return 0;
        }
    else {
        return 1;
        }
    }


void drawUnits() {


    // draw paths first, under units
    for( int i=0; i<numUnits && showMoves; i++ ) {
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
        

        if( getUnitMoveVisible( i ) &&
            gameUnit[i].mDest != gameUnit[i].mRegion ) {
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
        
        intPair pos = getUnitCurrentPosition( i );
        
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


    startNewSpriteLayer();

    // mark any bribed units
    for( int i=0; i<numUnits; i++ ) {
        if( gameUnit[i].mTotalSalary < gameUnit[i].mTotalBribe ) {
            
            char drawMarker = true;
            
            if( i < 3 ) {
                // one of our units!  Should we let the player know about this?

                // only if bribing unit has been compromised by us

                int bribingUnit = gameUnit[i].mLastBribingUnit;
                
                if( gameUnit[bribingUnit].mTotalSalary < 
                    gameUnit[bribingUnit].mTotalBribe ) {
                    
                    // bribing unit has been bribed!

                    drawMarker = true;
                    }
                else {
                    drawMarker = false;
                    }
                }
            
                
            if( drawMarker ) {
                
                // color of marker (opposite)
                rgbaColor c;
                if( i < 3 ) {
                    c = enemyColor;
                    }
                else if( i<6 ) {
                    c = playerColor;
                    }
                
                intPair pos = getUnitCurrentPosition( i );
        
                // center
                pos.x -= unitSpriteW / 2;
                pos.y -= unitSpriteH;

                drawSprite( bribedMarkerSprite, 
                            pos.x, 
                            pos.y, 
                            c, gameUnit[i].mSelectable );
                }
            }
        }
    


    // draw any diamond bids


    // first markers
    for( int i=0; i<numUnits-1 && showMoves; i++ ) {

        if( getUnitMoveVisible( i ) &&
            gameUnit[i].mDest != getHomeRegion( i ) ) {
            
            rgbaColor c;
            if( i < 3 ) {
                c = playerRegionColor;
                }
            else if( i<6 ) {
                c = enemyRegionColor;
                }

            intPair end = getUnitBidPosition( i );

            drawSprite( bidSprite, end.x - bidW / 2, end.y - bidH / 2,
                        c );
             
            }
        }
    startNewSpriteLayer();
    // then amounts
    for( int i=0; i<numUnits-1 && showMoves; i++ ) {

        if( getUnitMoveVisible( i ) &&
            gameUnit[i].mDest != getHomeRegion( i ) ) {
            
            
            intPair end = getUnitBidPosition( i );
            
            char *bidString = autoSprintf( "%d", gameUnit[i].mBid );

            font8->drawString( bidString, 
                               end.x + 8, 
                               end.y - 3,
                               black, 
                               alignRight );

            font8->drawString( "$", 
                               end.x - 9, 
                               end.y - 4,
                               black, 
                               alignLeft );

            delete [] bidString;
            }
        }
    


    // draw inspector bribes
    

    // first markers
    for( int i=0; i<numUnits-1 && showMoves; i++ ) {

        if( getUnitMoveVisible( i ) &&
            gameUnit[i].mDest == gameUnit[ numUnits - 1 ].mRegion
            &&
            gameUnit[i].mShowInspectorBribe ) {
            
            rgbaColor c;
            if( i < 3 ) {
                c = playerRegionColor;
                }
            else if( i<6 ) {
                c = enemyRegionColor;
                }

            intPair end = getUnitInspectorBribePosition( i );

            drawSprite( bidSprite, end.x - bidW / 2, end.y - bidH / 2,
                        c );
             
            }
        }
    startNewSpriteLayer();
    // then amounts
    for( int i=0; i<numUnits-1 && showMoves; i++ ) {

        if( getUnitMoveVisible( i ) &&
            gameUnit[i].mDest == gameUnit[ numUnits - 1 ].mRegion
            &&
            gameUnit[i].mShowInspectorBribe ) {
            
            
            intPair end = getUnitInspectorBribePosition( i );
            
            char *bidString = autoSprintf( "%d", gameUnit[i].mInspectorBribe );

            font8->drawString( bidString, 
                               end.x + 8, 
                               end.y - 3,
                               black, 
                               alignRight );

            font8->drawString( "$", 
                               end.x - 9, 
                               end.y - 4,
                               black, 
                               alignLeft );

            delete [] bidString;
            }
        }



    // draw salaries

    // first markers
    for( int i=0; i<numUnits-1; i++ ) {

        if( gameUnit[i].mShowSalaryPayment ) {
            
            // always being paid by player
            rgbaColor c = playerRegionColor;

            intPair end = getUnitSalaryPosition( i );

            drawSprite( bidSprite, end.x - bidW / 2, end.y - bidH / 2,
                        c );
             
            }
        }
    startNewSpriteLayer();
    // then amounts
    for( int i=0; i<numUnits-1; i++ ) {

        if( gameUnit[i].mShowSalaryPayment ) {
            
            
            intPair end = getUnitSalaryPosition( i );
            
            char *bidString = autoSprintf( "%d", 
                                           gameUnit[i].mLastSalaryPayment );

            font8->drawString( bidString, 
                               end.x + 8, 
                               end.y - 3,
                               black, 
                               alignRight );

            font8->drawString( "$", 
                               end.x - 9, 
                               end.y - 4,
                               black, 
                               alignLeft );

            delete [] bidString;
            }
        }




    // draw bribes

    // first markers
    for( int i=0; i<numUnits-1; i++ ) {

        if( gameUnit[i].mShowBribePayment ) {
            
            // always being paid by player
            rgbaColor c = playerRegionColor;

            intPair end = getUnitBribePosition( i );

            drawSprite( bidSprite, end.x - bidW / 2, end.y - bidH / 2,
                        c );
             
            }
        }
    startNewSpriteLayer();
    // then amounts
    for( int i=0; i<numUnits-1; i++ ) {

        if( gameUnit[i].mShowBribePayment ) {
            
            
            intPair end = getUnitBribePosition( i );
            
            char *bidString = autoSprintf( "%d", 
                                           gameUnit[i].mLastBribePayment );

            font8->drawString( bidString, 
                               end.x + 8, 
                               end.y - 3,
                               black, 
                               alignRight );

            font8->drawString( "$", 
                               end.x - 9, 
                               end.y - 4,
                               black, 
                               alignLeft );

            delete [] bidString;
            }
        }


    
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

int getUnitBid( int inUnit ) {
    return gameUnit[ inUnit ].mBid;
    }


void setUnitInspectorBribe( int inUnit, int inBribe ) {
    gameUnit[ inUnit ].mInspectorBribe = inBribe;
    }

int getUnitInspectorBribe( int inUnit ) {
    return gameUnit[ inUnit ].mInspectorBribe;
    }


void showInspectorBribe( int inUnit, char inShow ) {
    gameUnit[ inUnit ].mShowInspectorBribe = inShow;
    }





intPair getUnitBidPosition( int inUnit ) {
    intPair end = 
        getDiamondPositionInRegion( gameUnit[inUnit].mDest );

    end.y -= ( 6 + bidH / 2 );
    
    if( inUnit < 3 ) {
        end.x -= 10;
        }
    else {
        end.x += 10;
        }        

    return end;
    }



intPair getUnitInspectorBribePosition( int inUnit ) {
    // inspector pos
    intPair end = 
        getUnitPositionInRegion( gameUnit[inUnit].mDest, numUnits - 1 );

    
    if( inUnit < 3 ) {
        end.x -= (10 + 5);
        }
    else {
        end.x += (10 + 5);
        }
    end.y -= 6;
    
    
    return end;
    }



intPair getUnitSalaryPosition( int inUnit ) {
    // to right of unit
    intPair end = 
        getUnitPositionInRegion( gameUnit[inUnit].mRegion, inUnit );

    end.x += (10 + 5);
    end.y -= 6;
    return end;
    }

            

intPair getUnitBribePosition( int inUnit ) {
    // same as for salary
    return getUnitSalaryPosition( inUnit );
    }


void setMovePeeking( char inPeeking ) {
    peeking = inPeeking;
    }



char isAnyOpponentBribed() {
    for( int i=numPlayerUnits; i<numPlayerUnits*2; i++ ) {
        if( gameUnit[i].mTotalSalary < gameUnit[i].mTotalBribe ) {
            return true;
            }
        }
    return false;
    }



void showUnitMoves( char inShow ) {
    showMoves = inShow;
    }

void showAllUnitMoves( char inShow ) {
    showAllMoves = inShow;
    }






void executeUnitMoves() {
    executing = true;
    for( int i=0; i<numUnits; i++ ) {
        gameUnit[i].mExecutionStep = 0;
        
        // steps based on distance
        intPair start = 
            getUnitPositionInRegion( gameUnit[i].mRegion, i );
            
        intPair end = 
            getUnitPositionInRegion( gameUnit[i].mDest, i );
        
        int distance = intDistance( start, end );
        
        gameUnit[i].mNumExecutionSteps = distance / 2;
        }
    }


void stepUnits() {
    if( executing ) {
        
        char foundNext = false;
    
        for( int i=0; i<numUnits && !foundNext; i++ ) {
            if( gameUnit[i].mExecutionStep < gameUnit[i].mNumExecutionSteps ) {
                foundNext = true;
                gameUnit[i].mExecutionStep++;

                if( gameUnit[i].mExecutionStep == 
                    gameUnit[i].mNumExecutionSteps ) {
                    
                    // unit done moving, clear destination
                    gameUnit[i].mRegion = gameUnit[i].mDest;
                    }
                
                }
            }
        if( !foundNext ) {
            // done moving them all 
            for( int i=0; i<numUnits; i++ ) {                
                gameUnit[i].mExecutionStep = 0;
                }
            
            executing = false;
            }
        }
    
    }


char unitAnimationsDone() {
    return !executing;
    }





int getPlayerBribedInspector() {
    int highBribingUnit = -1;
    int highBribe = 0;
    
    for( int i=0; i<numPlayerUnits*2; i++ ) { 
        if( gameUnit[i].mRegion == gameUnit[numUnits - 1].mRegion
            &&
            gameUnit[i].mInspectorBribe > highBribe ) {
            
            highBribe = gameUnit[i].mInspectorBribe;
            
            highBribingUnit = i;
            }
        }
    
    if( highBribingUnit == -1 ) {
        return -1;
        }
    
    if( highBribingUnit < numPlayerUnits ) {
        return 0;
        }
    else {
        return 1;
        }
    }


