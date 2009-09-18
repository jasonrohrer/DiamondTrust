
#include "map.h"
#include "tga.h"
#include "platform.h"
#include "common.h"
#include "sprite.h"


#include <string.h>



// split 192-row sprite into two power-of-two-height parts

// top 128 rows
static int mapBackgroundTopSpriteID;
// bottom 64 rows
static int mapBackgroundBottomSpriteID;
static int bottomHalfOffset = 128;



static int mapRegionSpriteID[ numMapRegions ];

static intPair mapRegionOffset[ numMapRegions ];

static char mapRegionSelectable[ numMapRegions ];


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


    // first pixels in first row show region colors
    
    rgbaColor backgroundColor = mapRGBA[0];
    
    int i;
    for( i=0; i<numMapRegions; i++ ) {
        rgbaColor thisRegionColor = mapRGBA[ i + 1 ];
        
        regionColor[ i ] = thisRegionColor;
        
        mapRegionSelectable[ i ] = false;
        

        // hide key on map
        mapRGBA[ i + 1 ] = backgroundColor;
        

        // window around region
        int firstX = mapW;
        int firstY = mapH;
        int lastX = 0;
        int lastY = 0;
        
        for( int y=0; y<mapH; y++ ) {
            for( int x=0; x<mapW; x++ ) {
                
                if( equals( mapRGBA[ y * mapW + x ], thisRegionColor ) ) {
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
                    }
                }
            }
        int regionW = lastX - firstX + 1;
        int regionH = lastY - firstY + 1;
        
        int regionSpriteW = roundUpToPowerOfTwo( regionW );
        int regionSpriteH = roundUpToPowerOfTwo( regionH );
        
        // make sure expanded sprite is not hanging off the edge of the 
        // screen
        if( firstX + regionSpriteW > mapW ) {
            firstX = mapW - regionSpriteW;
            }
        if( firstY + regionSpriteH > mapH ) {
            firstY = mapH - regionSpriteH;
            }
        

        mapRegionOffset[i].x = firstX;
        mapRegionOffset[i].y = firstY;

        printOut( "Region %d offset = (%d,%d), size = (%d,%d)\n",
                  i, firstX, firstY, regionW, regionH );
        

        int numSpritePixels = regionSpriteW * regionSpriteH;
        
        rgbaColor *regionRGBA = new rgbaColor[ numSpritePixels ];
        
        // first set all to transparent
        memset( regionRGBA, 0, numSpritePixels * sizeof( rgbaColor ) );

        
        // copy out of map and into sprite
        for( int y=0; y<regionSpriteH; y++ ) {
            for( int x=0; x<regionSpriteW; x++ ) {
                
                int mapIndex = (y + firstY) * mapW
                    + x + firstX;
                int spriteIndex = y * regionSpriteW + x;
                
                regionRGBA[ spriteIndex ] = mapRGBA[ mapIndex ];
                }
            }

        // set any non-region-color pixels transparent
        rgbaColor clearColor = { 0, 0, 0, 0 };
        
        int j;
        for( j=0; j< numSpritePixels; j++ ) {
            if( ! equals( regionRGBA[j], thisRegionColor ) ) {        
                regionRGBA[j] = clearColor;
                }
            }
        
                
        mapRegionSpriteID[i] = addSprite( regionRGBA, regionSpriteW,
                                          regionSpriteH );
        
        delete [] regionRGBA;
        }

    // copy map w/ all region colors for click map
    mapRegionImage = new rgbaColor[ numMapPixels ];
    memcpy( mapRegionImage, mapRGBA, numMapPixels * sizeof( rgbaColor ) );


    // clear out all region colors for map background, leaving a thin
    // border of color
    for( i=0; i<numMapRegions; i++ ) {
        rgbaColor thisRegionColor = regionColor[ i ];
        
        // tag border, n-pixels wide
        for( int b=0; b<regionBorderWidth; b++ ) {
            char *tagMap = new char[ numMapPixels ];
            
            for( int y=1; y<mapH-1; y++ ) {
                for( int x=1; x<mapW-1; x++ ) {
                    
                    int j = y * mapW + x;
                    tagMap[j] = false;
                    if( equals( mapRGBA[ j ], thisRegionColor ) ) {
                        int upJ = j - mapW;
                        int downJ = j + mapW;
                        int leftJ = j - 1;
                        int rightJ = j + 1;

                        if( !equals( mapRGBA[ downJ ], thisRegionColor ) ||
                            !equals( mapRGBA[ upJ ], thisRegionColor ) ||
                            !equals( mapRGBA[ leftJ ], thisRegionColor ) ||
                            !equals( mapRGBA[ rightJ ], thisRegionColor ) ) {
                            
                            // edge
                            tagMap[j] = true;
                            }
                        }
                    }
                }
            
            for( int j=0; j<numMapPixels; j++ ) {
                if( tagMap[j] ) {
                    mapRGBA[j] = tagColor;
                    }
                }
            }
        

        // clear out center, ignoring tagged border
        int j;
        for( j=0; j<numMapPixels; j++ ) {
            if( equals( mapRGBA[j], thisRegionColor ) ) {
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
            if( equals( mapRGBA[j], tagColor ) ) {
                mapRGBA[j] = darkRegionColor;
                }
            }        }

    mapBackgroundTopSpriteID = addSprite( mapRGBA, mapW, bottomHalfOffset );

    rgbaColor *bottomHalfPointer = 
        &( mapRGBA[ bottomHalfOffset * mapW ] );
    
    mapBackgroundBottomSpriteID = addSprite( bottomHalfPointer, mapW, 
                                             mapH - bottomHalfOffset );

    delete [] mapRGBA;
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


