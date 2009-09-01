

#include "platform.h"
#include "tga.h"


#define NUM_DRAWN 2


rgbaColor drawColors[ NUM_DRAWN ];

int drawX[ NUM_DRAWN ];
int drawY[ NUM_DRAWN ];

int deltaX[ NUM_DRAWN ];
int deltaY[ NUM_DRAWN ];

int deltaFade[ NUM_DRAWN ];



int spriteID = -1;
int spriteIDB = -1;

static int loadSprite( char *inFileName, char inCornerTransparent = false ) {
    int returnID = -1;
    
    int fileDataSize;
    unsigned char *spriteFileData = readFile( inFileName, 
                                              &fileDataSize );
    if( spriteFileData != NULL ) {
        
        int width, height;
        
        rgbaColor *spriteRGBA = extractTGAData( spriteFileData, fileDataSize,
                                                &width, &height );
        
        if( spriteRGBA != NULL ) {
            
            if( inCornerTransparent ) {
                // use corner color as transparency
                spriteRGBA[0].a = 0;
                unsigned char tr, tg, tb;
                tr = spriteRGBA[0].r;
                tg = spriteRGBA[0].g;
                tb = spriteRGBA[0].b;

                int numPixels = width * height; 
                for( int i=0; i<numPixels; i++ ) {
                    if( spriteRGBA[i].r == tr 
                        &&
                        spriteRGBA[i].g == tg 
                        &&
                        spriteRGBA[i].b == tb ) {
                        
                        spriteRGBA[i].a = 0;
                        }
                    }
                }
            
                        
                        
                        
            returnID = addSprite( spriteRGBA, width, height );
            
            freeMem( spriteRGBA );
            }

        freeMem( spriteFileData );
        }
    return returnID;
    }



void gameInit() {
    spriteID = loadSprite( "testTexture.tga", true );
    spriteIDB = loadSprite( "testTexture2.tga" );
    
    int currentX = 0;
    int currentY = 0;
    
    for( int i=0; i<NUM_DRAWN; i++ ) {
        rgbaColor *c = &( drawColors[i] );
        c->r = 255;//(unsigned char)getRandom( 256 );
        c->g = 255;//(unsigned char)getRandom( 256 );
        c->b = 255;//(unsigned char)getRandom( 256 );
        c->a = (unsigned char)getRandom( 192 );

        deltaFade[i] = -8;
        //deltaFade[i] = 0;
        
        
        drawX[i] = (int)getRandom( 14 ) + 8;
        drawY[i] = (int)getRandom( 14 );
        
        if( i==0 ) {
            drawX[i] = 20;
            drawY[i] = 20;
            deltaFade[i] = 0;
            c->a = 255;
            }
        else {
            drawX[i] = 30;
            drawY[i] = 30;
            }
        deltaFade[i] = 0;
        c->a = 255;

        deltaX[i] = 1;//(int)getRandom( 2 ) - 1;
        deltaY[i] = 1;//(int)getRandom( 2 ) - 1;
        
        /*
        drawX[i] = currentX;
        drawY[i] = currentY;
        
        deltaX[i] = 0;
        deltaY[i] = 0;
        currentX += 3;
        currentY += 3;
        
        currentX += 16;
        //currentY += 3;
        if( currentX >= 256 ) {
            currentX = 0;
            currentY += 16;
            }
        */
        }
    
    }



int fade = 255;
int dFade = -8;
int x = 0;
int dX = 1;

void gameLoopTick() {

    for( int i=0; i<NUM_DRAWN; i++ ) {
        int f = drawColors[i].a;

        /*        
        f += deltaFade[i];
    
        if( f < 0 ) {
            f = 0;
            deltaFade[i] = 8;
            }
        if( f > 191 ) {
            f = 191;
            deltaFade[i] = -8;
            }
        */
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
