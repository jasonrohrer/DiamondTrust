#include "units.h"
#include "map.h"
#include "platform.h"
#include "common.h"
#include "tga.h"
#include "sprite.h"
#include "Font.h"
#include "colors.h"
#include "arrows.h"

#include "loading.h"

#include "music.h"


#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SimpleVector.h"

#include <assert.h>



Unit::Unit() 
        : mRegion( -1 ), mDest( -1 ), mSelectable( false ),
          mSpriteID( -1 ), mAnimationFrameNumber( 0 ), 
          mAnimationDirection( 1 ), mStepsUntilNextFrame( 0 ),
          mWavesLeftBeforeBreak( 4 ) {

    }


Unit gameUnit[ numUnits ];

static int activeUnit = -1;
static char peeking = false;
static char executing = true;
static char showMoves = false;
static char showAllMoves = false;

static int activeUnitSprite;

static int bidSprite, bidBorderSprite;
static int bidW, bidH;
extern Font *font8;

static int bribedMarkerSprite;



//static rgbaColor inspectorColor = {84, 84, 255, 255 };




int unitSpriteW;
int unitSpriteH;

int unitSpriteIDs[7];


int armSpriteW;
int armSpriteH;

int unitArmSpriteIDs[7][5];


int vehicleSpriteH;
int vehicleSpriteW;

// rotations in 11.25 degree steps
// one set per side (player, enemy, un)
int vehicleSpriteIDs[3][32];
int vehicleSpriteAngles[32];
const char *vehicleSpriteFileNames[3] = { "piper_rotated16.tga", 
                                          "jet_rotated16.tga", 
                                          "puma_rotated16.tga" };





void initUnits() {
    int i;
    
    int imageH;
    
    rgbaColor *playerUnitRGBA = readTGAFile( "playerUnits.tga",
                                             &unitSpriteW, &imageH );
    
    
    if( playerUnitRGBA == NULL ) {
        printOut( "Reading player unit sprite file failed.\n" );
        return;
        }


    rgbaColor *enemyUnitRGBA = readTGAFile( "enemyUnits.tga",
                                             &unitSpriteW, &imageH );
    
    
    if( enemyUnitRGBA == NULL ) {
        printOut( "Reading enemy unit sprite file failed.\n" );
        return;
        }


    // 1 pixel row between each sprite image
    unitSpriteH = ((imageH + 1) /  3 ) - 1;


    int inspectorSpriteW, inspectorSpriteH;
    
    rgbaColor *inspectorUnitRGBA = readTGAFile( "inspectorUnit.tga",
                                                &inspectorSpriteW, 
                                                &inspectorSpriteH );
    
    
    if( inspectorUnitRGBA == NULL ) {
        printOut( "Reading inspector unit sprite file failed.\n" );
        return;
        }




    int maskW, maskH;
    rgbaColor *unitMaskRGBA = readTGAFile( "unit_noArm_mask.tga",
                                           &maskW, &maskH );
    
    if( unitMaskRGBA == NULL || 
        maskW != unitSpriteW || maskH != unitSpriteH ) {
        
        printOut( "Reading unit sprite mask file failed.\n" );
        return;
        }


    int armImageH;
    
    rgbaColor *armMapRGBA = readTGAFile( "unit_arm_map.tga",
                                         &armSpriteW, &armImageH );
    
    if( armMapRGBA == NULL ) {
        printOut( "Reading arm map sprite file failed.\n" );
        return;
        }

    // 1 pixel row between each sprite image
    armSpriteH = ((armImageH + 1) / 5) - 1;

    int numArmSpritePixels = armSpriteW * armSpriteH;
    
    
    int sourceMapImageW, sourceMapImageH;
    
    rgbaColor *armSourceMapRGBA = readTGAFile( "unit_arm_sourceMap.tga",
                                               &sourceMapImageW,
                                               &sourceMapImageH );
    
    if( armSourceMapRGBA == NULL || 
        sourceMapImageW != unitSpriteW || sourceMapImageH != unitSpriteH ) {

        printOut( "Reading arm source map sprite file failed.\n" );
        return;
        }

    


    // decode arm map once into coordinates
    // source index plus 5 dest indices, one for each arm position
    // source index maps into source map, dest index map into full
    // arm sprite image (which contains 5 arm position images)
    // Each arm has 6 pixels in it that need to be mapped
    int sourceIndex[6];
    int destIndex[5][6];

    int numSourcePixels = sourceMapImageH * sourceMapImageW;
    for( i=0; i<6; i++ ) {

        // colors "listed" in first 6 pixels of image
        rgbaColor mapColor = armSourceMapRGBA[ i ];
        

        // search rest of map for it
        char found = false;
        for( int j=i+1; j<numSourcePixels &&!found; j++ ) {
            if( equals( armSourceMapRGBA[j], mapColor ) ) {
                found = true;
                
                sourceIndex[i] = j;
                }
            }
    
        if( !found ) {
            printOut( "Failed to map color %d in arm map\n", i );
            return;
            }
        
        // now find it in destination sub images
        for( int s=0; s<5; s++ ) {

            int indexOffset = s * (armSpriteH + 1) * armSpriteW;
            
            rgbaColor *subImage = 
                &( armMapRGBA[ indexOffset ] );


            applyCornerTransparency( subImage, armSpriteW * armSpriteH );

            
            found = false;
            for( int j=0; j<numArmSpritePixels &&!found; j++ ) {
                if( equals( subImage[j], mapColor ) ) {
                    found = true;
                
                    // map into full arm image
                    destIndex[s][i] = j + indexOffset;
                    }
                }

            // flag not found
            if( !found ) { 
                destIndex[s][i] = -1;
                }
            
            }
        
        }
    

        
    for( i=0; i<7; i++ ) {
        
        rgbaColor *rgbaSource = playerUnitRGBA;
        if( i > 2 ) {
            rgbaSource = enemyUnitRGBA;
            }
        if( i > 5 ) {
            rgbaSource = inspectorUnitRGBA;
            }
        
        
        
        rgbaColor *subImage;

        if( i<6 ) {
            subImage = 
                &( rgbaSource[ ( i % 3 ) * (unitSpriteH + 1) * unitSpriteW ] );
            }
        else {
            // only one frame in inspector image
            subImage = inspectorUnitRGBA;
            }
        
        applyCornerTransparency( subImage, unitSpriteW * unitSpriteH );


        // roll player color right into sprite, replacing gray areas
        
        rgbaColor subColor = playerColor;
        if( i > 2 ) {
            subColor = enemyColor;
            }
        if( i > 5 ) {
            subColor = inspectorColor;
            }
        
        

        int numSubPixels = unitSpriteH * unitSpriteW;
        
        int p;
        for( p=0; p<numSubPixels; p++ ) {
            if( subImage[ p ].r == 
                subImage[ p ].g
                &&
                subImage[ p ].r == 
                subImage[ p ].b ) {
                // gray
                
                // modulate by player color (make gray a shade of player
                // color)
                subImage[p].r = 
                    (unsigned char)( (subImage[p].r * subColor.r) / 255 );
                subImage[p].g = 
                    (unsigned char)( (subImage[p].g * subColor.g) / 255 );
                subImage[p].b = 
                    (unsigned char)( (subImage[p].b * subColor.b) / 255 );
                }
            }
        
                


        // construct arm sprites using map

        rgbaColor *armMapWorking = new rgbaColor[ armImageH * armSpriteW ];
        memcpy( armMapWorking, armMapRGBA, 
                armImageH * armSpriteW * sizeof( rgbaColor ) );
        
        // 6 source pixels
        for( int s=0; s<6; s++ ) {
            rgbaColor spritePixelColor = subImage[ sourceIndex[s] ];
        
            // place color into 5 destination arm images
            for( int d=0; d<5; d++ ) {
                if( destIndex[d][s] >= 0 ) {
                    armMapWorking[ destIndex[d][s] ] = spritePixelColor;
                    }
                // else no mapping for this pixel
                }    
            }
        
        // our arm sprites are ready to be split and added
        
        for( int a=0; a<5; a++ ) {
            rgbaColor * armSubImage = 
                &( armMapWorking[ a * (armSpriteH + 1) * armSpriteW ] );

            unitArmSpriteIDs[i][a] = 
                addSprite( armSubImage, armSpriteW, armSpriteH );
            }
        delete [] armMapWorking;


        // now clear out an area of unit sprite that will be replaced by
        // arm sprite

        for( p=0; p<numSubPixels; p++ ) {
            if( unitMaskRGBA[p].r == 0 ) {
                subImage[p].a = 0;
                }
            }
        

        unitSpriteIDs[i] = 
            addSprite( subImage, unitSpriteW, unitSpriteH );
        }

    delete [] playerUnitRGBA;
    delete [] enemyUnitRGBA;
    delete [] inspectorUnitRGBA;
    delete [] unitMaskRGBA;
    delete [] armMapRGBA;
    delete [] armSourceMapRGBA;
    
    updateLoadingProgress();
    



    printOut( "Initing unit vehicle sprites\n" );

    // add them all to the same set to conserve RAM
    // only one vehicle sprite is on the screen at a time
    int vehicleSpriteSet = createSpriteSet();


    // construct vehicle sprites
    for( int t=0; t<3; t++ ) {
        printOut( "Loading from file %s\n", vehicleSpriteFileNames[t] );
        
        rgbaColor *vehicleRGBA = readTGAFile( vehicleSpriteFileNames[t],
                                              &vehicleSpriteW, &imageH );
    
    
        if( vehicleRGBA == NULL ) {
            printOut( "Reading jet sprite file failed.\n" );
            return;
            }


        // 1 pixel row between each sprite image
        // 8 base images in sprite file (first quadrant) that we rotate
        // by 90 degrees to fill other 3 quadrants
        vehicleSpriteH = ((imageH + 1) /  8 ) - 1;

        if( vehicleSpriteH != vehicleSpriteW ) {
            printOut( "Only square vehicle images supported.\n" );
            return;
            }
    
    
        // each increment is 11.25
        int angleSumTimes100 = 0;
        int angleIncrementTimes100 = 1125;

        for( int r=0; r<8; r++ ) {
            printOut( "Vehicle angle %d\n", r );
            
            // round down
            vehicleSpriteAngles[r] = angleSumTimes100 / 100;
            angleSumTimes100 += angleIncrementTimes100;
        
        

            rgbaColor * vehicleSubImage = 
                &( vehicleRGBA[ r * (vehicleSpriteH + 1) * vehicleSpriteW ] );

            int numVehiclePixels = vehicleSpriteW * vehicleSpriteH;
        
            applyCornerTransparency( vehicleSubImage, numVehiclePixels );

            // first quadrant
            vehicleSpriteIDs[t][r] = 
                addSprite( vehicleSubImage, vehicleSpriteW, vehicleSpriteH,
                           vehicleSpriteSet );

            // accumulate rotations here
            rgbaColor *vehicleAccumRotated = new rgbaColor[ numVehiclePixels ];
            memcpy( vehicleAccumRotated, vehicleSubImage, 
                    numVehiclePixels * sizeof( rgbaColor ) );
        
        

            // remaining quadrants
            for( int q=1; q<4; q++ ) {
            
                int spriteIndex = q * 8 + r;
            
                vehicleSpriteAngles[ spriteIndex ] = 
                    vehicleSpriteAngles[r] + 90 * q;
            
                // copy accum into this, rotating by 90 again
                rgbaColor *vehicleRotated = new rgbaColor[ numVehiclePixels ];
            
                // source and dest x,y
                for( int sY=0; sY<vehicleSpriteH; sY++ ) {
                    int dX = (vehicleSpriteH - 1) - sY;

                    for( int sX=0; sX<vehicleSpriteH; sX++ ) {
                    
                        int dY = sX;
                    
                        vehicleRotated[ dY * vehicleSpriteW + dX ] =
                            vehicleAccumRotated[ sY * vehicleSpriteW + sX ];
                        }
                    }
            
                // save into accume
                memcpy( vehicleAccumRotated, vehicleRotated, 
                        numVehiclePixels * sizeof( rgbaColor ) );
            
                delete [] vehicleRotated;

                vehicleSpriteIDs[t][ spriteIndex ] = 
                    addSprite( vehicleAccumRotated, 
                               vehicleSpriteW, vehicleSpriteH, 
                               vehicleSpriteSet );
                }
        
            delete [] vehicleAccumRotated;
            
            }
        delete [] vehicleRGBA;
        
        }
    
    updateLoadingProgress();



    // all player units start at home


    for( i=0; i<6; i++ ) {
        gameUnit[ i ].mSpriteID = unitSpriteIDs[ i ];
        
        for( int a=0; a<5; a++ ) {
            gameUnit[ i ].mArmSpriteID[a] = unitArmSpriteIDs[i][a];
            }
        }
    
    

    // inspector 
    // start in random producing region
    gameUnit[ 6 ].mRegion = (int)getRandom( numMapRegions - 2 ) + 2;
    
    gameUnit[ 6 ].mSpriteID = unitSpriteIDs[ 6 ];
    

    // inspector has arms too, though only cell phone one is used
    for( int a=0; a<5; a++ ) {
        gameUnit[ 6 ].mArmSpriteID[a] = unitArmSpriteIDs[6][a];
        }

    resetUnits();


    

    activeUnitSprite = loadSprite( "activeUnitHalo.tga", true );
    
    bidSprite = loadSprite( "bid.tga", &bidW, &bidH, true );
    bidBorderSprite = loadSprite( "bidBorder.tga", &bidW, &bidH, true );
    
    bribedMarkerSprite = loadSprite( "bribedMarker.tga", true );





    // now pre-build arrow sprites

    // first player avoids region 1
    // second avoids region 0
    // inspector avoids 0 and 1
    for( int u=0; u<numUnits; u++ ) {
        

        for( i=0; i<numMapRegions; i++ ) {
            
            char skip = false;
            
            if( u < numPlayerUnits && i == 1 ) {
                skip = true;
                }
            else if( u >= numPlayerUnits && u < numPlayerUnits * 2 &&
                     i == 0 ) {
                skip = true;
                }
            else if( u >= numPlayerUnits * 2 && i < 2 ) {
                // inspector
                skip = true;
                }            

            if( ! skip ) {
                
                intPair start = getUnitPositionInRegion( i, u );
            
                for( int j=0; j<numMapRegions; j++ ) {                    
                    if( j != i ) {
                        
                        skip = false;

                        if( u < numPlayerUnits && j == 1 ) {
                            skip = true;
                            }
                        else if( u >= numPlayerUnits && 
                                 u < numPlayerUnits * 2 &&
                                 j == 0 ) {
                            skip = true;
                            }
                        else if( u >= numPlayerUnits * 2 && j < 2 ) {
                            // inspector
                            skip = true;
                            }

                        if( !skip ) {
                            
                            intPair end = getUnitPositionInRegion( j, u );
                    
                            printOut( "Adding arrow from %d,%d to %d,%d\n",
                                      start.x, start.y, end.x, end.y );
                            
                            buildArrow( start, end );
                            }
                        
                        }
                
                    }
                }
            }

        updateLoadingProgress();
        }

             
    }



void resetUnits() {
    
    executing = false;
    
    // player
    gameUnit[ 0 ].mRegion = 0;
    gameUnit[ 1 ].mRegion = 0;
    gameUnit[ 2 ].mRegion = 0;

    // enemy
    gameUnit[ 3 ].mRegion = 1;
    gameUnit[ 4 ].mRegion = 1;
    gameUnit[ 5 ].mRegion = 1;

    int i;
    
    for( i=0; i<6; i++ ) {
        gameUnit[ i ].mShowInspectorBribe = false;

        gameUnit[ i ].mTotalSalary = 0;
        gameUnit[ i ].mLastSalaryPayment = 0;
        gameUnit[ i ].mShowSalaryPayment = false;
        
        gameUnit[ i ].mTotalBribe = 0;
        gameUnit[ i ].mLastBribePayment = 0;
        gameUnit[ i ].mShowBribePayment = false;
        gameUnit[ i ].mMinKnownTotalBribe = 0;
        gameUnit[ i ].mLastBribingUnit = -1;

        gameUnit[ i ].mEnemyContactSinceBribeKnown = false;
        }

    // destinations -- nowhere
    // bids -- none
    for( i=0; i<numUnits; i++ ) {
        gameUnit[ i ].mDest = gameUnit[ i ].mRegion;
        gameUnit[ i ].mBid = 0;
        gameUnit[ i ].mExecutionStep = 0;
        gameUnit[ i ].mFlying = 0;
        
        gameUnit[ i ].mNumDiamondsHeld = 0;
        gameUnit[ i ].mTripCost = 0;
        }
    }




void freeUnits() {
    }


Unit *getUnit( int inUnit ) {
    assert( inUnit >= 0 );
    
    return &( gameUnit[ inUnit ] );
    }



static char getUnitMoveVisible( int inUnit ) {
    assert( inUnit >= 0 );

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



static intPair getUnitCurrentPosition( int inUnit ) {
    assert( inUnit >= 0 );

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


static int getHomeRegion( int inUnit ) {
    assert( inUnit >= 0 );

    if( inUnit < numPlayerUnits ) {
        return 0;
        }
    else {
        return 1;
        }
    }



static char isBribeStatusVisible( int inUnit ) {
    assert( inUnit >= 0 );

    int i = inUnit;
    char visible = false;
    
    if( gameUnit[i].mTotalSalary < gameUnit[i].mTotalBribe ) {
            
        visible = true;
        
        if( i < 3 ) {
            // one of our units!  Should we let the player know about this?
            
            // only if what we last knew our bribe to be (the minimum possible
            // that it could be now, given that it only goes up) is higher
            // than our salary (which we know for sure)

            if( gameUnit[i].mTotalSalary >= gameUnit[i].mMinKnownTotalBribe ) {
                visible = false;
                }
            }
        }

    return visible;
    }



static void drawUnitSprite( int inUnit, intPair inPos, 
                            unsigned char inAlpha = 255 ) {
    assert( inUnit >= 0 );

    int i = inUnit;
    
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
    
    // ignore color for now
    c = white;
    c.a = inAlpha;
    
        
    // center
    intPair drawPos = inPos;
    drawPos.x -= unitSpriteW / 2;
    drawPos.y -= unitSpriteH;
    
    
    /*
      // used for waving animation

    int animFrame = gameUnit[i].mAnimationFrameNumber;
    if( activeUnit == i ) {
        // talking on phone sprite
        animFrame = 4;
        }
    */

    // disable flashing, try waving based on animation frame number
    //drawSprite( unitSpriteIDs[ animFrame ], 
    drawSprite( gameUnit[i].mSpriteID, 
                drawPos.x, 
                drawPos.y, 
                c, false /*gameUnit[i].mSelectable*/ );


    intPair armSpritePos = inPos;
    armSpritePos.x -= armSpriteW / 2 + 2;
    armSpritePos.y -= armSpriteH + 3; 

    int armToDraw;
    
    if( i == activeUnit ) {
        // cell phone arm
        armToDraw = 4;
        }
    else {
        armToDraw = gameUnit[i].mAnimationFrameNumber;
        }
    
    
    drawSprite( gameUnit[i].mArmSpriteID[ armToDraw ], 
                armSpritePos.x, 
                armSpritePos.y, 
                c, false );
    
    
    }



static void drawUnitBribedMarker( int inUnit, intPair inPos, 
                                  unsigned char inAlpha = 255  ) {
    assert( inUnit >= 0 );

    int i = inUnit;

    // color of marker (opposite)
    rgbaColor c;
    if( i < 3 ) {
        c = enemyColor;
        }
    else if( i<6 ) {
        c = playerColor;
        }
            
    c.a = inAlpha;
    
    // center
    inPos.x -= unitSpriteW / 2;
    inPos.y -= unitSpriteH;
            
    drawSprite( bribedMarkerSprite, 
                inPos.x, 
                inPos.y, 
                c, false );
    }



static void drawUnitPaymentNumber( intPair inPos, int inValue ) {
    char *bidString = autoSprintf( "%d", inValue );

    font8->drawString( bidString, 
                       inPos.x + 8, 
                       inPos.y - 3,
                       black, 
                       alignRight );
    
    font8->drawString( "$", 
                       inPos.x - 9, 
                       inPos.y - 3,
                       black, 
                       alignLeft );
    
    delete [] bidString;
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
            //c = white;
            // color needed with new procedural arrows
            c = inspectorColor;
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

            // try drawing arrow
            drawArrow( start, end, c );

            // new layer for each line drawn
            startNewSpriteLayer();
            
            }
        }



    // now draw units on top
    
    for( int i=0; i<numUnits; i++ ) {
        if( ! gameUnit[i].mFlying ) {
            intPair pos = getUnitCurrentPosition( i );
            
            drawUnitSprite( i, pos );
        
            if( activeUnit == i ) {
                rgbaColor c;
                if( i < 3 ) {
                    c = playerColor;
                    }
                else if( i<6 ) {
                    c = enemyColor;
                    }
                else {
                    c = inspectorColor;
                    }            
                
                // center
                pos.x -= unitSpriteW / 2;
                pos.y -= unitSpriteH;
                
                // tweak a bit
                pos.y += 2;
                
                drawSprite( activeUnitSprite, 
                            pos.x, 
                            pos.y, 
                            c, false );
                }
            }
        }



    
    

    startNewSpriteLayer();

    // mark any bribed units
    for( int i=0; i<numUnits; i++ ) {

        if( isBribeStatusVisible( i ) &&
            // don't draw bribed marker on top of moving vehicle
            ! gameUnit[i].mFlying ) {
            
            drawUnitBribedMarker( i, getUnitCurrentPosition( i ) );
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
            
            drawUnitPaymentNumber( end, gameUnit[i].mBid );
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
            
            drawUnitPaymentNumber( end, gameUnit[i].mInspectorBribe );
            }
        }



    
    
    // now draw any moving vehicles on top of stationary units
    // (they fly over top, not under, even on top of diamond bids and 
    //   inspector bribes)
    startNewSpriteLayer();
    
    for( int i=0; i<numUnits; i++ ) {
        if( gameUnit[i].mFlying ) {
            intPair pos = getUnitCurrentPosition( i );

            // draw vehicle instead
            int vehicleType;
            rgbaColor c;
            if( i < 3 ) {
                vehicleType = 0;
                c = playerColor;
                }
            else if( i<6 ) {
                vehicleType = 1;
                c = enemyColor;
                }
            else {
                vehicleType = 2;
                c = inspectorColor;
                }            
            
            // center
            pos.x -= vehicleSpriteW / 2;
            pos.y -= vehicleSpriteH / 2;
                
            // tweak a bit
            pos.y += 2;
            

            // fade in and out at begining and end
            int step = gameUnit[i].mExecutionStep;
            int numSteps = gameUnit[i].mNumExecutionSteps;
            
            if( step < 8 ) {
                int alpha = step * 32;
                if( alpha > 255 ) {
                    alpha = 255;
                    }
                c.a = (unsigned char)alpha;
                }
            else if( step > numSteps - 8 ) {
                int alpha = (numSteps - step) * 32;
                if( alpha > 255 ) {
                    alpha = 255;
                    }
                c.a = (unsigned char)alpha;
                }
            

            drawSprite( vehicleSpriteIDs
                          [ vehicleType ]
                          [ gameUnit[i].mVehicleSpriteIndex ], 
                        pos.x, 
                        pos.y, 
                        c, false );

            startNewSpriteLayer();

            // show unit fading out in start region and
            // fading back in in end region as vehicle fades in/out
            if( c.a < 255 ) {

                // invert
                unsigned char alpha = (unsigned char)( 255 - c.a );

                intPair anchorPos;
                
                if( step < 8 ) {
                    anchorPos = 
                        getUnitPositionInRegion( gameUnit[i].mRegion, i );
                    }
                else {    
                    anchorPos = 
                        getUnitPositionInRegion( gameUnit[i].mDest, i );
                    }
                
                drawUnitSprite( i, anchorPos, alpha );            
                
                if( isBribeStatusVisible( i ) ) {
                    startNewSpriteLayer();
                    drawUnitBribedMarker( i, anchorPos, alpha );
                    }
                    
                
                }
            
                
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
            drawUnitPaymentNumber( end, gameUnit[i].mLastSalaryPayment );
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
            drawUnitPaymentNumber( end, gameUnit[i].mLastBribePayment );
            }
        }


    
    }



void drawUnit( int inUnit, int inX, int inY ) {
    assert( inUnit >= 0 );

    intPair pos = { inX, inY };
    drawUnitSprite( inUnit, pos );
    
    startNewSpriteLayer();
    
    if( isBribeStatusVisible( inUnit ) ) {
        drawUnitBribedMarker( inUnit, pos );
        }
    }


void drawUnitBribe( int inUnit, int inX, int inY ) {
    assert( inUnit >= 0 );
    
    int i = inUnit;
    
    intPair end = { inX, inY };
    
    // color of marker (opposite)
    rgbaColor c;
    if( i < 3 ) {
        c = enemyRegionColor;
        }
    else if( i<6 ) {
        c = playerRegionColor;
        }

    drawSprite( bidSprite, end.x - bidW / 2, end.y - bidH / 2, c );

    startNewSpriteLayer();

    drawUnitPaymentNumber( end, gameUnit[i].mTotalBribe );
    }



void drawBidBorder( int inX, int inY ) {
    drawBlinkingSprite( bidBorderSprite, inX - bidW / 2, inY - bidH / 2, 
                        white );
    }




// custom function so that we can disable entropy-inducing calls
// to getRandom when testing
static inline unsigned int getRandomFake( unsigned int inMax ) {    
    return inMax / 2;
    }

#define getRandomCustom getRandom
// for testing:
//#define getRandomCustom getRandomFake




static int getArmHoldDownTime() {
    return (int)( getRandomCustom( 60 ) + 30 );
    }



void setUnitSelectable( int inUnit, char inSelectable ) {
    assert( inUnit >= 0 );

    int i = inUnit;
    
    char oldSelectable = gameUnit[ i ].mSelectable;

    gameUnit[ i ].mSelectable = inSelectable;

    if( inSelectable && ! oldSelectable ) {
        // selectable just turned on

        // hold down arm for random time, a bit shorter than between-wave
        // hold-downs
        gameUnit[i].mStepsUntilNextFrame = getArmHoldDownTime() - 30;
        }
    else if( ! inSelectable ) {
        // back to arm down, not waving
        gameUnit[i].mAnimationFrameNumber = 0;
        gameUnit[i].mAnimationDirection = 1;
        gameUnit[i].mStepsUntilNextFrame = 0;
        }
    
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



int getChosenUnit( int inClickX, int inClickY, char inOnlySelectable ) {
    for( int i=0; i<numUnits; i++ ) {
        if( ! inOnlySelectable || 
            gameUnit[i].mSelectable ) {

            intPair pos = getUnitPositionInRegion( gameUnit[i].mRegion, i );

            // make horizontal click radius less than full, square sprite width
            // Most of the square is transparent, and the full squares 
            // sometimes overlap due to unit map positioning, which makes
            // it hard to accurately click/differentiate nearby units.
            // try 1/4 instead of 1/2 width for radius
            // True sprite width is about 4.5 pixels... this 1/4 width
            // extends several pixels beyond that
            // Actually, hard code the horizontal radius here, to give
            // us more control (Waving arm, or highlighted sprite, sticks
            // out beyond click radius of 1/4 width)

            // ALSO:
            // hardcode true sprite height here, because unitSpriteH includes
            // way too much headroom, making sprite click boxes overlap
            // in weird ways when sprites are packed closely in the home
            // region
            // True height is 18.  Add one extra pixel above that.
            if( inClickY < pos.y &&
                inClickY > pos.y - 19 &&
                inClickX > pos.x - 10 &&
                inClickX < pos.x + 8 ) {
                
                // hit on unit's sprite
                return i;
                }

            if( gameUnit[i].mShowSalaryPayment ||
                gameUnit[i].mShowBribePayment ) {
                
                intPair salaryPos = getUnitSalaryPosition( i );
                
                if( inClickY > salaryPos.y - 7 &&
                    inClickY < salaryPos.y + 7 &&
                    inClickX > salaryPos.x - 14 &&
                    inClickX < salaryPos.x + 14 ) {
                    
                    // hit on unit's salary display
                    return i;
                    }
                }
            
                        
            }
        }
                    
    return -1;
    }



int getChosenBidMarker( int inClickX, int inClickY ) {
    for( int i=0; i<numUnits; i++ ) {
        if( getUnitMoveVisible( i ) &&
            gameUnit[i].mDest != getHomeRegion( i ) ) {
            
            intPair bidPos = getUnitBidPosition( i );

            if( inClickY > bidPos.y - 7 &&
                inClickY < bidPos.y + 7 &&
                inClickX > bidPos.x - 14 &&
                inClickX < bidPos.x + 14 ) {
                
                // hit on unit's bid display
                return i;
                }
            }
        }
    return -1;
    }



int getChosenInspectorBribeMarker( int inClickX, int inClickY ) {
    for( int i=0; i<numUnits; i++ ) {

        if( getUnitMoveVisible( i ) &&
            gameUnit[i].mDest == gameUnit[ numUnits - 1 ].mRegion ) {

            // ignore marker hit if marker isn't visible
            char hitMarker = false;

            if( gameUnit[i].mShowInspectorBribe ) {
                
                intPair bribePos = getUnitInspectorBribePosition( i );

                hitMarker = ( inClickY > bribePos.y - 7 &&
                              inClickY < bribePos.y + 7 &&
                              inClickX > bribePos.x - 14 &&
                              inClickX < bribePos.x + 14 );
                }
            
            if( hitMarker 
                ||
                // also count a direct click on the inspector himself,
                // even if inspector's bribe marker is not showing
                getChosenUnit( inClickX, inClickY, false ) == numUnits - 1 ) {
                
                // hit on unit's bribe display
                return i;
                }
            }
        }
    return -1;
    }





int getUnitRegion( int inUnit ) {
    assert( inUnit >= 0 );

    return gameUnit[ inUnit ].mRegion;
    }


void setUnitDestination( int inUnit, int inRegion ) {
    assert( inUnit >= 0 );

    gameUnit[ inUnit ].mDest = inRegion;
    }


int getUnitDestination( int inUnit ) {
    assert( inUnit >= 0 );
    
    return gameUnit[ inUnit ].mDest;
    }


void setUnitBid( int inUnit, int inBid ) {
    assert( inUnit >= 0 );
    
    gameUnit[ inUnit ].mBid = inBid;
    }

int getUnitBid( int inUnit ) {
    assert( inUnit >= 0 );
    
    return gameUnit[ inUnit ].mBid;
    }


void setUnitInspectorBribe( int inUnit, int inBribe ) {
    assert( inUnit >= 0 );
    
    gameUnit[ inUnit ].mInspectorBribe = inBribe;
    }

int getUnitInspectorBribe( int inUnit ) {
    assert( inUnit >= 0 );
    
    return gameUnit[ inUnit ].mInspectorBribe;
    }


void showInspectorBribe( int inUnit, char inShow ) {
    assert( inUnit >= 0 );
    
    gameUnit[ inUnit ].mShowInspectorBribe = inShow;
    }





intPair getUnitBidPosition( int inUnit ) {
    assert( inUnit >= 0 );
    
    intPair end = 
        getDiamondPositionInRegion( gameUnit[inUnit].mDest );

    end.y -= ( 8 + bidH / 2 );
    
    if( inUnit < 3 ) {
        end.x -= 13;
        }
    else {
        end.x += 13;
        }        

    return end;
    }



intPair getUnitInspectorBribePosition( int inUnit ) {
    assert( inUnit >= 0 );
    
    // inspector pos
    intPair end = 
        getUnitPositionInRegion( gameUnit[inUnit].mDest, numUnits - 1 );

    
    if( inUnit < 3 ) {
        end.x -= (14 + 5);
        }
    else {
        end.x += (14 + 4);
        }
    end.y -= 7;
    
    
    return end;
    }



intPair getUnitSalaryPosition( int inUnit ) {
    assert( inUnit >= 0 );

    // to right of unit
    intPair end = 
        getUnitPositionInRegion( gameUnit[inUnit].mRegion, inUnit );

    end.x += (14 + 4);
    end.y -= 9;
    return end;
    }

            

intPair getUnitBribePosition( int inUnit ) {
    assert( inUnit >= 0 );

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



char isAnyPlayerUnitKnownBribed() {
    for( int i=0; i<numPlayerUnits; i++ ) {
        if( isBribeStatusVisible( i ) ) {
            return true;
            }
        }
    return false;
    }



char isOpponentHomeBribed() {
    for( int i=numPlayerUnits; i<numPlayerUnits*2; i++ ) {
        if( gameUnit[i].mTotalSalary < gameUnit[i].mTotalBribe 
            &&
            gameUnit[i].mRegion == 1 ) {

            return true;
            }
        }
    return false;
    }



char isPlayerHomeKnownBribed() {
    for( int i=0; i<numPlayerUnits; i++ ) {
        if( isBribeStatusVisible( i )
            && 
            gameUnit[i].mRegion == 0 ) {

            // at home and we know it's bribed
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
                gameUnit[i].mFlying = true;

                if( gameUnit[i].mExecutionStep == 0 ) {
                    // just starting
                    // compute which vehicle sprite to use

                    // based on angle of trip

                    intPair start = 
                        getUnitPositionInRegion( gameUnit[i].mRegion, i );
                    intPair end = 
                        getUnitPositionInRegion( gameUnit[i].mDest, i );
                    
                    intPair delta = subtract( end, start );

                    int tripAngle = intArctan2( delta.x, delta.y );
                    
                    // compare to all sprite angles to find closest
                    
                    // special case for a=0; because that's also 360 degrees
                    int closestIndex = 0;
                    int closestDelta = 
                        intAbs( vehicleSpriteAngles[0] - tripAngle );
                    int altDelta = intAbs( 360 - tripAngle );
                    if( altDelta < closestDelta ) {
                        closestDelta = altDelta;
                        }

                    // rest of angles can be handled as general case
                    for( int a=1; a<32; a++ ) {
                        int delta = intAbs( vehicleSpriteAngles[a] - tripAngle );
                    
                        if( delta < closestDelta ) {
                            closestDelta = delta;
                            closestIndex = a;
                            }
                        }
                    
                    gameUnit[i].mVehicleSpriteIndex = closestIndex;
                    
                    //printOut( "vehicle sprite = %d\n", 
                    //          gameUnit[i].mVehicleSpriteIndex );
                    }
                

                gameUnit[i].mExecutionStep++;
                
                if( gameUnit[i].mExecutionStep == 
                    gameUnit[i].mNumExecutionSteps ) {
                    
                    // unit done moving, clear destination
                    gameUnit[i].mRegion = gameUnit[i].mDest;
                    gameUnit[i].mFlying = false;
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
    
    // step animations for selectable units
    for( int i=0; i<numUnits; i++ ) {
        if( gameUnit[i].mSelectable ) {
            if( gameUnit[i].mStepsUntilNextFrame == 0 ) {
                // next frame

                if( gameUnit[i].mAnimationFrameNumber == 0 ) {
                    // just starting a fresh wave

                    // pick how many times to wave
                    gameUnit[i].mWavesLeftBeforeBreak = 
                        (int)( getRandomCustom( 4 ) + 3 );
                    }
                
                
                gameUnit[i].mAnimationFrameNumber += 
                    gameUnit[i].mAnimationDirection;
                if( gameUnit[i].mAnimationDirection == 1 &&
                    gameUnit[i].mAnimationFrameNumber == 3 ) {
                    
                    // reverse
                    gameUnit[i].mAnimationDirection *= -1;
                    }
                else if( gameUnit[i].mAnimationDirection == -1 &&
                    gameUnit[i].mAnimationFrameNumber == 1 ) {
                    
                    // reverse
                    gameUnit[i].mAnimationDirection *= -1;

                    // finished another wave
                    gameUnit[i].mWavesLeftBeforeBreak--;
                    }

                gameUnit[i].mStepsUntilNextFrame = 
                    (int)( getRandomCustom( 4 ) + 4 );

                if( gameUnit[i].mWavesLeftBeforeBreak == 0 ) {
                    
                    // take a break
                    gameUnit[i].mAnimationFrameNumber = 0;
                    gameUnit[i].mAnimationDirection = 1;
                    
                    // hold arm down for a randon while
                        
                    gameUnit[i].mStepsUntilNextFrame = getArmHoldDownTime();
                    }
                }
            else {
                // count down
                gameUnit[i].mStepsUntilNextFrame -= 1;
                }
            }
        else {
            // stop waving
            gameUnit[i].mAnimationFrameNumber = 0;
            gameUnit[i].mAnimationDirection = 1;
            gameUnit[i].mStepsUntilNextFrame = 0;
            }
        }
    

    stepArrows();
    }


char unitAnimationsDone() {
    return !executing;
    }





int getPlayerBribedInspector() {
    int highBribingUnit = -1;
    int highBribe = 0;
    
    for( int i=0; i<numPlayerUnits*2; i++ ) { 
        if( gameUnit[i].mRegion == gameUnit[numUnits - 1].mRegion ) {
            
            int bribe = gameUnit[i].mInspectorBribe;
            if( bribe > highBribe ) {
            
                highBribe = gameUnit[i].mInspectorBribe;
            
                highBribingUnit = i;
                }
            else if( bribe == highBribe ) {
                // ties with previous high bribe
                // cancels it
                highBribingUnit = -1;
                }
            
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



char isAnyUnitPayable() {
    
    // home units
    int i;
    for( i=0; i<numPlayerUnits*2; i++ ) {
        if( getUnitRegion( i ) == 0 || 
            getUnitRegion( i ) == 1 ) {
            
            return true;
            }
        }
    // opponent units in shared regions with opponent
    for( i=0; i<numPlayerUnits*2; i++ ) {
        int unitRegion = getUnitRegion( i );
        
        if( unitRegion > 1 ) {
            
            for( int j=0; j<numPlayerUnits*2; j++ ) {
                if( j!=i && 
                    getUnitRegion( j ) == unitRegion ) {
                    
                    return true;
                    }
                }
            }
        }

    return false;
    }



char isAnyUnitBuyingDiamonds() {

    char found = false;
    
    int inspectorRegion = getUnitRegion( numUnits - 1 );
    
    for( int i=0; i<numPlayerUnits*2 && !found; i++ ) {
        
        int unitRegion = getUnitRegion( i );
        int unitBid = getUnitBid( i );
        char highest = true;
        
        if( unitRegion != inspectorRegion &&
            unitBid > 0 && 
            getDiamondsInRegion( unitRegion ) > 0 ) {
            
            // look for other unit in this region
            for( int j=0; j<numPlayerUnits*2; j++ ) {
                if( j != i ) {
                    if( getUnitRegion( j ) == unitRegion ) {
                        if( getUnitBid( j ) >= unitBid ) {
                            highest = false;
                            }
                        }
                    }
                }
            
            if( highest ) {
                found = true;
                }
            }
        }
    
    return found;
    }



char isAnyUnitDepositingDiamonds() {

    char found = false;
    
    
    for( int i=0; i<numPlayerUnits*2 && !found; i++ ) {
            
        Unit *u = getUnit( i );
        
        int unitRegion = getUnitRegion( i );
        int unitDiamonds = u->mNumDiamondsHeld;

        if( ( unitRegion == 0 || unitRegion == 1 ) 
            &&
            unitDiamonds > 0 ) {
                
            found = true;
            }
        }
        
    return found;
    }



char isAnyConfiscationNeeded() {
    
    int inspectorRegion = getUnitRegion( numUnits - 1 );
    
    for( int i=0; i<numPlayerUnits*2; i++ ) {
            
        Unit *u = getUnit( i );

        if( getUnitRegion( i ) == inspectorRegion 
            &&
            u->mNumDiamondsHeld > 0 ) {
            
            return true;
            }
        }
        
    return false;
    }




char *getUnitPositionString() {
    SimpleVector<char> accumVector;
    
    char *headerString = "Units at: ";
    
    accumVector.push_back( headerString, (int)strlen( headerString ) );
    

    for( int i=0; i<numUnits; i++ ) {
        int region = getUnitRegion( i );
        
        char regionDigit = (char)( '0' + region );
        
        accumVector.push_back( regionDigit );
        accumVector.push_back( ',' );
        }
    
    return accumVector.getElementString();
    }

