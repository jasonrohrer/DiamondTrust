

#include "platform.h"
#include "tga.h"


#define NUM_DRAWN 500


rgbaColor drawColors[ NUM_DRAWN ];

int drawX[ NUM_DRAWN ];
int drawY[ NUM_DRAWN ];

int deltaX[ NUM_DRAWN ];
int deltaY[ NUM_DRAWN ];

int deltaFade[ NUM_DRAWN ];



int spriteID = -1;
int spriteIDB = -1;

static int loadSprite( char *inFileName ) {
    int returnID = -1;
    
    int fileDataSize;
    unsigned char *spriteFileData = readFile( inFileName, 
                                              &fileDataSize );
    if( spriteFileData != NULL ) {
        
        int width, height;
        
        rgbaColor *spriteRGBA = extractTGAData( spriteFileData, fileDataSize,
                                                &width, &height );
        
        if( spriteRGBA != NULL ) {
            returnID = addSprite( spriteRGBA, width, height );
            
            freeMem( spriteRGBA );
            }

        freeMem( spriteFileData );
        }
    return returnID;
    }



void gameInit() {
    spriteID = loadSprite( "testTexture.tga" );
    spriteIDB = loadSprite( "testTexture2.tga" );
    
    for( int i=0; i<NUM_DRAWN; i++ ) {
        rgbaColor *c = &( drawColors[i] );
        c->r = (unsigned char)getRandom( 256 );
        c->g = (unsigned char)getRandom( 256 );
        c->b = (unsigned char)getRandom( 256 );
        c->a = 255;//(unsigned char)getRandom( 256 );

        //deltaFade[i] = -8;
        deltaFade[i] = 0;
        
        drawX[i] = (int)getRandom( 256 );
        drawY[i] = (int)getRandom( 192 );
        
        deltaX[i] = (int)getRandom( 2 ) - 1;
        deltaY[i] = (int)getRandom( 2 ) - 1;
        }
    
    }



int fade = 255;
int dFade = -8;
int x = 0;
int dX = 1;

void gameLoopTick() {

    for( int i=0; i<NUM_DRAWN; i++ ) {
        int f = drawColors[i].a;
        
        f += deltaFade[i];
    
        if( f < 0 ) {
            f = 0;
            deltaFade[i] = 8;
            }
        if( f > 255 ) {
            f = 255;
            deltaFade[i] = -8;
            }
        drawColors[i].a = (unsigned char)f;
        
        
        drawX[i] += deltaX[i];
    
        if( drawX[i] < 0 ) {
            drawX[i] = 0;
            deltaX[i] = 1;
            }
        if( drawX[i] > 255 ) {
            drawX[i] = 255;
            deltaX[i] = -1;
            }
        
        drawY[i] += deltaY[i];

        if( drawY[i] < 0 ) {
            drawY[i] = 0;
            deltaY[i] = 1;
            }
        if( drawY[i] > 191 ) {
            drawY[i] = 191;
            deltaY[i] = -1;
            }
        }
    
    }



void drawTopScreen() {
    

    for( int i=0; i<NUM_DRAWN; i++ ) {
        drawSprite( spriteID, drawX[i], drawY[i], drawColors[i] );
        }
    }


void drawBottomScreen() {
    for( int i=0; i<NUM_DRAWN; i++ ) {
        drawSprite( spriteIDB, drawX[i], drawY[i], drawColors[i] );
        }
    }
