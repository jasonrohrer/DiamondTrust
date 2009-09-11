
#include "map.h"
#include "tga.h"
#include "platform.h"
#include "common.h"
#include "sprite.h"


#include <string.h>



static int mapBackgroundSpriteID;



static int mapRegionSpriteID[ numMapRegions ];

static intPair mapRegionOffset[ numMapRegions ];

static char mapRegionSelectable[ numMapRegions ];


// for checking region clicks
static rgbaColor *mapRegionImage;
static int mapW, mapH;

static rgbaColor regionColor[ numMapRegions ];



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
        
        mapRegionOffset[i].x = firstX;
        mapRegionOffset[i].y = firstY;

        
        int regionSpriteW = roundUpToPowerOfTwo( regionW );
        int regionSpriteH = roundUpToPowerOfTwo( regionH );
        
        printOut( "Region %d offset = (%d,%d), size = (%d,%d)\n",
                  i, firstX, firstY, regionW, regionH );
        

        int numSpritePixels = regionSpriteW * regionSpriteH;
        
        rgbaColor *regionRGBA = new rgbaColor[ numSpritePixels ];
        
        // first set all to transparent
        memset( regionRGBA, 0, numSpritePixels * sizeof( rgbaColor ) );

        
        // copy out of map and into sprite
        for( int y=0; y<regionH; y++ ) {
            for( int x=0; x<regionW; x++ ) {
                
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


    // clear out all region colors for map background
    for( i=0; i<numMapRegions; i++ ) {
        rgbaColor thisRegionColor = regionColor[ i ];
        
        for( int j=0; j<numMapPixels; j++ ) {
            if( equals( mapRGBA[j], thisRegionColor ) ) {
                mapRGBA[j] = backgroundColor;
                }
            }
        }

    mapBackgroundSpriteID = addSprite( mapRGBA, mapW, mapH );
    }



void freeMap() {
    delete [] mapRegionImage;
    }


static rgbaColor white = { 255, 255, 255, 255 }; 

void drawMap() {
    drawSprite( mapBackgroundSpriteID, 0, 0, white );
    
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
            return i;
            }
        }

    return -1;
    }


