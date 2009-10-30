
#include "map.h"
#include "tga.h"
#include "platform.h"
#include "common.h"
#include "sprite.h"
#include "Font.h"
#include "colors.h"
#include "gameStats.h"

#include "minorGems/util/stringUtils.h"


#include <string.h>
#include <stdio.h>



// split 192-row sprite into two power-of-two-height parts

// top 128 rows
static int mapBackgroundTopSpriteID;
// bottom 64 rows
static int mapBackgroundBottomSpriteID;
static int bottomHalfOffset = 128;

static int mapNamesTopSpriteID;
static int mapNamesBottomSpriteID;

static int mapPaperTopSpriteID;
static int mapPaperBottomSpriteID;



int diamondSpriteID;
int diamondSpriteW, diamondSpriteH;
int diamondBorderSpriteID;

int vaultSpriteID;
int vaultSpriteW, vaultSpriteH;



static int mapRegionSpriteID[ numMapRegions ];

static intPair mapRegionOffset[ numMapRegions ];

static char mapRegionSelectable[ numMapRegions ];

static int mapRegionDiamondCount[ numMapRegions ];
static int mapRegionDiamondRate[ numMapRegions ];



// positions of units in regions
static intPair mapRegionUnitPosition[ numMapRegions ][ 3 ];

// positions of diamond marker in regions
static intPair mapRegionDiamondPosition[ numMapRegions ];

static char mapRegionDiamondAccumulating[ numMapRegions ];


// positions of vault marker in regions
static intPair mapRegionVaultPosition[ numMapRegions ];


// for checking region clicks
static rgbaColor *mapRegionImage;
static int mapW, mapH;

static rgbaColor regionColor[ numMapRegions ];


#define regionBorderWidth 3
// colors unused in map image, used for tagging during processing
static rgbaColor tagColor[ regionBorderWidth ] = 
{ { 255, 0, 255, 255 },
  { 255, 0, 254, 255 },
  { 255, 0, 253, 255 } }; 




void initMap() {
    rgbaColor *mapRGBA = readTGAFile( "angola_map.tga",
                                      &mapW, &mapH );
    
    
    if( mapRGBA == NULL
        ||
        mapW < 256 ) {
        
        printOut( "Reading map file failed.\n" );
        return;
        }
    
    
    int numMapPixels = mapW * mapH;

    
    unsigned int *mapPixelInts = new unsigned int[ numMapPixels ];
    for( int j=0; j<numMapPixels; j++ ) {
        mapPixelInts[j] = toInt( mapRGBA[j] );
        }
    

    intPair regionFirstCorner[ numMapRegions ];
    intPair regionLastCorner[ numMapRegions ];
    

    // first pixels in first row show region colors
    
    rgbaColor backgroundColor = mapRGBA[0];
    unsigned int backgroundColorInt = toInt( backgroundColor );
    
    
    rgbaColor unitMarkerColor = mapRGBA[ numMapRegions + 1 ];
    unsigned int unitMarkerColorInt = toInt( unitMarkerColor );
    // clear it
    mapRGBA[ numMapRegions + 1 ] = backgroundColor;

    rgbaColor diamondMarkerColor = mapRGBA[ numMapRegions + 2 ];
    unsigned int diamondMarkerColorInt = toInt( diamondMarkerColor );
    // clear it
    mapRGBA[ numMapRegions + 2 ] = backgroundColor;

    rgbaColor vaultMarkerColor = mapRGBA[ numMapRegions + 3 ];
    unsigned int vaultMarkerColorInt = toInt( vaultMarkerColor );
    // clear it
    mapRGBA[ numMapRegions + 3 ] = backgroundColor;


    int i;
    for( i=0; i<numMapRegions; i++ ) {
        rgbaColor thisRegionColor = mapRGBA[ i + 1 ];
        unsigned int thisRegionColorInt = toInt( thisRegionColor );
        
        regionColor[ i ] = thisRegionColor;
        
        mapRegionSelectable[ i ] = false;
        

        // hide key on map
        mapRGBA[ i + 1 ] = backgroundColor;
        mapPixelInts[ i + 1 ] = backgroundColorInt;
        

        // window around region
        int firstX = mapW;
        int firstY = mapH;
        int lastX = 0;
        int lastY = 0;
        
        int numMarkersFound = 0;
        

        for( int y=0; y<mapH; y++ ) {
            for( int x=0; x<mapW; x++ ) {
                
                int pixIndex = y * mapW + x;
                
                if( mapPixelInts[ pixIndex ] == thisRegionColorInt ) {

                    if( y < firstY ) {
                        firstY = y;
                        }
                    if( x < firstX ) {
                        firstX = x;
                        }
                    if( y > lastY ) {
                        lastY = y;
                        }
                    if( x > lastX ) {
                        lastX = x;
                        }
                    
                    if( mapPixelInts[ pixIndex + 1 ] == unitMarkerColorInt ) {
                        // marker found
                        
                        // clear it
                        mapPixelInts[ pixIndex + 1 ] = thisRegionColorInt;
                        mapRGBA[ pixIndex + 1 ] = thisRegionColor;
                        
                        // remember it
                        mapRegionUnitPosition[ i ][ numMarkersFound ].x
                            = x + 1;
                        mapRegionUnitPosition[ i ][ numMarkersFound ].y
                            = y;
                        numMarkersFound++;
                        }
                    else if( mapPixelInts[ pixIndex + 1 ] == 
                             diamondMarkerColorInt ) {

                        // marker found
                        
                        // clear it
                        mapPixelInts[ pixIndex + 1 ] = thisRegionColorInt;
                        mapRGBA[ pixIndex + 1 ] = thisRegionColor;
                        
                        // remember it
                        mapRegionDiamondPosition[ i ].x = x + 1;
                        mapRegionDiamondPosition[ i ].y = y;
                        }
                    else if( mapPixelInts[ pixIndex + 1 ] == 
                             vaultMarkerColorInt ) {

                        // marker found
                        
                        // clear it
                        mapPixelInts[ pixIndex + 1 ] = thisRegionColorInt;
                        mapRGBA[ pixIndex + 1 ] = thisRegionColor;
                        
                        // remember it
                        mapRegionVaultPosition[ i ].x = x + 1;
                        mapRegionVaultPosition[ i ].y = y;
                        }

                    }
                }
            }
        int regionW = lastX - firstX + 1;
        int regionH = lastY - firstY + 1;
        
        regionFirstCorner[i].x = firstX;
        regionFirstCorner[i].y = firstY;
        regionLastCorner[i].x = lastX;
        regionLastCorner[i].y = lastY;
        
        

        int regionSpriteW = roundUpToPowerOfTwo( regionW );
        int regionSpriteH = roundUpToPowerOfTwo( regionH );
        
        // make sure expanded sprite is not hanging off the edge of the 
        // screen
        int spriteX = firstX;
        int spriteY = firstY;
        
        if( spriteX + regionSpriteW > mapW ) {
            spriteX = mapW - regionSpriteW;
            }
        if( spriteY + regionSpriteH > mapH ) {
            spriteY = mapH - regionSpriteH;
            }
        

        mapRegionOffset[i].x = spriteX;
        mapRegionOffset[i].y = spriteY;

        printOut( "Region %d offset = (%d,%d), size = (%d,%d)\n",
                  i, spriteX, spriteY, regionW, regionH );
        

        int numSpritePixels = regionSpriteW * regionSpriteH;
        
        rgbaColor *regionRGBA = new rgbaColor[ numSpritePixels ];
        
        // first set all to transparent
        // don't do this... we set the outside pixels to trans below
        //memset( regionRGBA, 0, numSpritePixels * sizeof( rgbaColor ) );

        
        // copy out of map and into sprite
        for( int y=0; y<regionSpriteH; y++ ) {
            
            // memcpy whole row
            int mapRowStart = (y + spriteY) * mapW + spriteX;
            int spriteRowStart = y * regionSpriteW;
            
            memcpy( &( regionRGBA[ spriteRowStart ] ),
                    &( mapRGBA[ mapRowStart ] ),
                    regionSpriteW * sizeof( rgbaColor ) );
            
            /*
            for( int x=0; x<regionSpriteW; x++ ) {
                
                int mapIndex = (y + spriteY) * mapW
                    + x + spriteX;
                int spriteIndex = y * regionSpriteW + x;
                
                regionRGBA[ spriteIndex ] = mapRGBA[ mapIndex ];
                }
            */
            }

        // set any non-region-color pixels transparent
        rgbaColor clearColor = { 0, 0, 0, 0 };
        
        int j;
        for( j=0; j< numSpritePixels; j++ ) {
            if( ! equals( regionRGBA[j], thisRegionColor ) ) {        
                regionRGBA[j] = clearColor;
                }
            }
        
        //printOut( "Adding region sprite\n" );
        mapRegionSpriteID[i] = addSprite( regionRGBA, regionSpriteW,
                                          regionSpriteH );
        //printOut( "Done adding region sprite\n" );
        
        delete [] regionRGBA;
        }

    // copy map w/ all region colors for click map
    mapRegionImage = new rgbaColor[ numMapPixels ];
    memcpy( mapRegionImage, mapRGBA, numMapPixels * sizeof( rgbaColor ) );


    // clear out all region colors for map background, leaving a thin
    // border of color
    char *tagMap = new char[ numMapPixels ];
    
    
    
    unsigned int tagColorInt[ regionBorderWidth ];
    for( i=0; i<regionBorderWidth; i++ ) {
        tagColorInt[i] = toInt( tagColor[i] );
        }
    
    

    for( i=0; i<numMapRegions; i++ ) {
        rgbaColor thisRegionColor = regionColor[ i ];
        unsigned int thisRegionColorInt = toInt( thisRegionColor );
        
        printOut( "Creating border in background for region %d\n", i );
        printf( "Test\n" );
        
        int firstX = regionFirstCorner[i].x;
        int firstY = regionFirstCorner[i].y;
        int lastX = regionLastCorner[i].x;
        int lastY = regionLastCorner[i].y;
        

        // tag border, n-pixels wide
        for( int b=0; b<regionBorderWidth; b++ ) {
            memset( tagMap, false, (unsigned int)numMapPixels );

            for( int y=firstY; y<=lastY; y++ ) {
                for( int x=firstX; x<=lastX; x++ ) {
                    
                    int j = y * mapW + x;
                    tagMap[j] = false;
                    if( mapPixelInts[ j ] == thisRegionColorInt ) {
                        int upJ = j - mapW;
                        int downJ = j + mapW;
                        int leftJ = j - 1;
                        int rightJ = j + 1;

                        if( mapPixelInts[ downJ ] != thisRegionColorInt ||
                            mapPixelInts[ upJ ] != thisRegionColorInt ||
                            mapPixelInts[ leftJ ] != thisRegionColorInt ||
                            mapPixelInts[ rightJ ] != thisRegionColorInt ) {
                            
                            // edge
                            tagMap[j] = true;
                            }
                        }
                    }
                }
            
            for( int j=0; j<numMapPixels; j++ ) {
                if( tagMap[j] ) {
                    mapRGBA[j] = tagColor[b];
                    mapPixelInts[j] = tagColorInt[b];
                    }
                }
            }
        
        

        // clear out center, ignoring tagged border
        int j;
        for( j=0; j<numMapPixels; j++ ) {
            if( mapPixelInts[j] == thisRegionColorInt ) {
                mapRGBA[j] = backgroundColor;
                }
            }

        // restore border color
        // make it darker near edge, and fade to white
        rgbaColor darkRegionColor = thisRegionColor;
        darkRegionColor.r /= 3;
        darkRegionColor.g /= 3;
        darkRegionColor.b /= 3;
            
        for( int b=0; b<regionBorderWidth; b++ ) {
            
            rgbaColor borderRingColor;
            borderRingColor.r = 
                ( (regionBorderWidth - b) * darkRegionColor.r +
                  b * white.r ) / regionBorderWidth;
            borderRingColor.g = 
                ( (regionBorderWidth - b) * darkRegionColor.g +
                  b * white.g ) / regionBorderWidth;
            borderRingColor.b = 
                ( (regionBorderWidth - b) * darkRegionColor.b +
                  b * white.b ) / regionBorderWidth;
            
            borderRingColor.a = thisRegionColor.a;
            

            for( j=0; j<numMapPixels; j++ ) {
                if( mapPixelInts[j] == tagColorInt[b] ) {
                    mapRGBA[j] = borderRingColor;
                    
                    // doesn't matter what we set int to, as long as it's
                    // not tag color
                    // done with region color, use that
                    mapPixelInts[j] = thisRegionColorInt;
                    }
                }
            } 
        }
   
    delete [] tagMap;
    

    mapBackgroundTopSpriteID = addSprite( mapRGBA, mapW, bottomHalfOffset );

    rgbaColor *bottomHalfPointer = 
        &( mapRGBA[ bottomHalfOffset * mapW ] );
    
    mapBackgroundBottomSpriteID = addSprite( bottomHalfPointer, mapW, 
                                             mapH - bottomHalfOffset );

    delete [] mapRGBA;
    delete [] mapPixelInts;





    int namesW, namesH;
    
    rgbaColor *namesRGBA = readTGAFile( "map_names.tga",
                                        &namesW, &namesH );

    if( namesRGBA == NULL
        ||
        namesW < 256 ) {
        
        printOut( "Reading map names file failed.\n" );
        return;
        }

    applyCornerTransparency( namesRGBA, namesW * namesH );
    
    mapNamesTopSpriteID = addSprite( namesRGBA, namesW, bottomHalfOffset );

    bottomHalfPointer = 
        &( namesRGBA[ bottomHalfOffset * namesW ] );
    
    mapNamesBottomSpriteID = addSprite( bottomHalfPointer, namesW, 
                                        namesH - bottomHalfOffset );

    delete [] namesRGBA;




    int paperW, paperH;
    
    rgbaColor *paperRGBA = readTGAFile( "paper.tga",
                                        &paperW, &paperH );

    if( paperRGBA == NULL
        ||
        paperW < 256 ) {
        
        printOut( "Reading map paper file failed.\n" );
        return;
        }

    mapPaperTopSpriteID = addSprite( paperRGBA, paperW, bottomHalfOffset );

    bottomHalfPointer = 
        &( paperRGBA[ bottomHalfOffset * paperW ] );
    
    mapPaperBottomSpriteID = addSprite( bottomHalfPointer, paperW, 
                                        paperH - bottomHalfOffset );

    delete [] paperRGBA;

    


    diamondSpriteID = loadSprite( "diamond.tga", 
                                  &diamondSpriteW, &diamondSpriteH,
                                  true );
    diamondBorderSpriteID = loadSprite( "diamondBorder.tga", true );

    vaultSpriteID = loadSprite( "vault.tga", 
                                &vaultSpriteW, &vaultSpriteH,
                                true );


    mapRegionDiamondRate[ 0 ] = 0;
    mapRegionDiamondRate[ 1 ] = 0;
    
    mapRegionDiamondRate[ 2 ] = 1;
    mapRegionDiamondRate[ 3 ] = 1;
    mapRegionDiamondRate[ 4 ] = 1;
    mapRegionDiamondRate[ 5 ] = 1;
    mapRegionDiamondRate[ 6 ] = 2;
    // last region swaps between 1 and 2 after each accumulation
    mapRegionDiamondRate[ 7 ] = 1;

    for( i=0; i<numMapRegions; i++ ) {
        mapRegionDiamondCount[i] = 44;

        mapRegionDiamondAccumulating[i] = false;
        }
    }



void freeMap() {
    delete [] mapRegionImage;
    }



extern Font *font16;


unsigned char diamondBorderAlpha = 255;


void drawMap() {
    // paper under it all
    
    drawSprite( mapPaperTopSpriteID, 0, 0, white );
    drawSprite( mapPaperBottomSpriteID, 0, bottomHalfOffset, white );
    
    startNewSpriteLayer();
    
    
    rgbaColor backgroundColor = white;
    // map image gets more and more faint as game wears on
    // starts at 75%, fades down to 50%
    backgroundColor.a = 128 + ( 64 * getMonthsLeft() ) / 8;
    


    drawSprite( mapBackgroundTopSpriteID, 0, 0, backgroundColor );
    drawSprite( mapBackgroundBottomSpriteID, 0, bottomHalfOffset, 
                backgroundColor );
    
    startNewSpriteLayer();    

    rgbaColor regionColor = white;
    
    int i;
    for( i=0; i<numMapRegions; i++ ) {

        if( mapRegionSelectable[i] ) {
            // darker so flashing more visible
            regionColor.a = 224;
            }
        else {
            // transparent so border shows when not flashing
            regionColor.a = 160;
            //regionColor.a = 128;
            }
        
        
        drawSprite( mapRegionSpriteID[i], 
                    mapRegionOffset[i].x, mapRegionOffset[i].y,
                    regionColor,
                    mapRegionSelectable[ i ] );
        }
    
    startNewSpriteLayer();

    
    // names, transparent
    rgbaColor nameColor = white;
    nameColor.a = 64;
    drawSprite( mapNamesTopSpriteID, 0, 0, nameColor );
    drawSprite( mapNamesBottomSpriteID, 0, bottomHalfOffset, nameColor );



    // next vaults (only in home regions
    for( i=0; i<2; i++ ) {
        drawSprite( vaultSpriteID, 
                    mapRegionVaultPosition[i].x - vaultSpriteW/2, 
                    mapRegionVaultPosition[i].y - vaultSpriteH/2, 
                    white );
        }

    // next diamonds (only producing regions)
    for( i=2; i<numMapRegions; i++ ) {
        drawSprite( diamondSpriteID, 
                    mapRegionDiamondPosition[i].x - diamondSpriteW/2, 
                    mapRegionDiamondPosition[i].y - diamondSpriteH/2, 
                    white );
        if( mapRegionDiamondAccumulating[i] ) {
            rgbaColor borderColor = white;
            borderColor.a = diamondBorderAlpha;
            
            drawSprite( diamondBorderSpriteID, 
                        mapRegionDiamondPosition[i].x - diamondSpriteW/2, 
                        mapRegionDiamondPosition[i].y - diamondSpriteH/2, 
                        borderColor );
            }        
        }

    startNewSpriteLayer();
    
    // next diamond counters
    for( i=2; i<numMapRegions; i++ ) {
        char *countString = autoSprintf( "%d", mapRegionDiamondCount[i] );
        
        font16->drawString( countString, 
                            mapRegionDiamondPosition[i].x, 
                            mapRegionDiamondPosition[i].y - 6,
                            diamondPurple, 
                            alignCenter );

        delete [] countString;
        }
    }



void setRegionSelectable( int inRegion, char inSelectable ) {
    mapRegionSelectable[ inRegion ] = inSelectable;
    }



void setAllRegionsNotSelectable() {
    for( int r=0; r<numMapRegions; r++ ) {
        setRegionSelectable( r, false );
        }
    }



int getChosenRegion( int inClickX, int inClickY ) {
    rgbaColor clickColor = mapRegionImage[ inClickY * mapW + inClickX ];
    
    for( int i=0; i<numMapRegions; i++ ) {
        if( equals( regionColor[i], clickColor ) ) {
            if( mapRegionSelectable[i] ) {
                return i;
                }
            else {
                return -1;
                }
            }
        }

    return -1;
    }



intPair getUnitPositionInRegion( int inRegion, int inUnitNumber ) {
    intPair badReturn = { -1, -1 };
    
    

    // home regions
    if( inRegion == 0 ) {
        if( inUnitNumber < 3 ) {
            return mapRegionUnitPosition[ inRegion ][ inUnitNumber ];
            }
        }
    else if( inRegion == 1 ) {
        if( inUnitNumber > 2 && inUnitNumber < 6 ) {
            return mapRegionUnitPosition[ inRegion ][ inUnitNumber - 3 ];
            }
        }
    
    if( inRegion < 2 ) {
        return badReturn;
        }
    

    // production regions
    if( inUnitNumber < 3 ) {
        // player 
        return mapRegionUnitPosition[ inRegion ][ 0 ];
        }
    else if( inUnitNumber < 6 ) {
        // enemy
        return mapRegionUnitPosition[ inRegion ][ 1 ];
        }
    else {
        // inspector
        return mapRegionUnitPosition[ inRegion ][ 2 ];
        }
    }



intPair getDiamondPositionInRegion( int inRegion ) {
    return mapRegionDiamondPosition[ inRegion ];
    }


intPair getVaultPositionInRegion( int inRegion ) {
    return mapRegionVaultPosition[ inRegion ];
    }




static int numToAccumulate[numMapRegions];
static int numAccumulated[numMapRegions];


void accumulateDiamondsStart() {
    for( int i=0; i<numMapRegions; i++ ) {
        numToAccumulate[i] = mapRegionDiamondRate[i];
        numAccumulated[i] = 0;
        mapRegionDiamondAccumulating[i] = false;
        }    

    // toggle between 1 and 2 (used for next Start)
    mapRegionDiamondRate[ 7 ] %= 2;
    mapRegionDiamondRate[ 7 ] ++;
    }


void stepDiamondBorderFade() {
    unsigned char stepSize = 8;
    
    if( diamondBorderAlpha > stepSize ) {
        diamondBorderAlpha -= stepSize;
        }
    else {
        diamondBorderAlpha = 0;
        }
    }


char accumulateDiamondsStep() {
    int i;
    
    for( i=0; i<numMapRegions; i++ ) {
        if( numToAccumulate[i] > numAccumulated[i] ) {
            numAccumulated[i]++;
            mapRegionDiamondCount[i]++;
         
            // previous region done
            // safe to do this, because first producing region has i=2
            mapRegionDiamondAccumulating[i-1] = false;
            
            mapRegionDiamondAccumulating[i] = true;

            diamondBorderAlpha = 255;
            
            // not done
            return false;
            }
        }
    
    
    // get here, done

    for( i=0; i<numMapRegions; i++ ) {
        mapRegionDiamondAccumulating[i] = false;
        }    

    return true;
    }



int getDiamondsInRegion( int inRegion ) {
    return mapRegionDiamondCount[ inRegion ];
    }


void decrementDiamonds( int inRegion ) {
    mapRegionDiamondCount[ inRegion ]--;
    }

