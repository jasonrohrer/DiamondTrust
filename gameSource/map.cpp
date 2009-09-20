
#include "map.h"
#include "tga.h"
#include "platform.h"
#include "common.h"
#include "sprite.h"


#include <string.h>
#include <stdio.h>



// split 192-row sprite into two power-of-two-height parts

// top 128 rows
static int mapBackgroundTopSpriteID;
// bottom 64 rows
static int mapBackgroundBottomSpriteID;
static int bottomHalfOffset = 128;



static int mapRegionSpriteID[ numMapRegions ];

static intPair mapRegionOffset[ numMapRegions ];

static char mapRegionSelectable[ numMapRegions ];


// positions of units in regions
static intPair mapRegionUnitPosition[ numMapRegions ][ 3 ];



// for checking region clicks
static rgbaColor *mapRegionImage;
static int mapW, mapH;

static rgbaColor regionColor[ numMapRegions ];


// a color unused in map image, used for tagging during processing
static rgbaColor tagColor = { 255, 0, 255, 255 }; 

static int regionBorderWidth = 3;



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
                            = x;
                        mapRegionUnitPosition[ i ][ numMarkersFound ].y
                            = y;
                        numMarkersFound++;
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
    
    
    
    unsigned int tagColorInt = toInt( tagColor );
    

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
                for( int x=firstX; x<lastX; x++ ) {
                    
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
                    //mapRGBA[j] = tagColor;
                    mapPixelInts[j] = tagColorInt;
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
        // make it darker
        rgbaColor darkRegionColor = thisRegionColor;
        darkRegionColor.r /= 2;
        darkRegionColor.g /= 2;
        darkRegionColor.b /= 2;
                
        for( j=0; j<numMapPixels; j++ ) {
            if( mapPixelInts[j] == tagColorInt ) {
                mapRGBA[j] = darkRegionColor;

                // doesn't matter what we set int to, as long as it's
                // not tag color
                // done with region color, use that
                mapPixelInts[j] = thisRegionColorInt;
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
    }



void freeMap() {
    delete [] mapRegionImage;
    }


static rgbaColor white = { 255, 255, 255, 255 }; 

void drawMap() {
    drawSprite( mapBackgroundTopSpriteID, 0, 0, white );
    drawSprite( mapBackgroundBottomSpriteID, 0, bottomHalfOffset, white );
    
    startNewSpriteLayer();
    
    for( int i=0; i<numMapRegions; i++ ) {
        if( mapRegionSelectable[ i ] ) {
            
            drawBlinkingSprite( mapRegionSpriteID[i], 
                                mapRegionOffset[i].x, mapRegionOffset[i].y,
                                white );
            }
        else {
            drawSprite( mapRegionSpriteID[i], 
                        mapRegionOffset[i].x, mapRegionOffset[i].y,
                        white );
            }
        }
    }



void setRegionSelectable( int inRegion, char inSelectable ) {
    mapRegionSelectable[ inRegion ] = inSelectable;
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
