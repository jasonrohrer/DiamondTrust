

#include "platform.h"
#include "tga.h"
#include "Font.h"
#include "minorGems/util/stringUtils.h"

#define NUM_DRAWN 60


rgbaColor drawColors[ NUM_DRAWN ];

int indexToTouch = 0;

int drawX[ NUM_DRAWN ];
int drawY[ NUM_DRAWN ];

int deltaX[ NUM_DRAWN ];
int deltaY[ NUM_DRAWN ];

int deltaFade[ NUM_DRAWN ];



int spriteID = -1;
int spriteIDB = -1;

int parentSpriteID = -1;
int childSpriteID = -1;

int parentButtonX = 10;
int parentButtonY = 150;
int childButtonX = 10;
int childButtonY = 170;

char buttonsVisible = true;

char *serverAddress = NULL;


Font *font8;
Font *font16;



 
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
            
            delete [] spriteRGBA;
            }

        delete [] spriteFileData;
        }
    return returnID;
    }



static char isInside( int inX, int inY, 
                      int inStartX, int inStartY, int inWidth, int inHeight ) {
    if( inStartX <= inX && inX < inStartX + inWidth 
        &&
        inStartY <= inY && inY < inStartY + inHeight ) {
        return true;
        }
    return false;
    }



void gameInit() {
    spriteID = loadSprite( "testTexture.tga", true );
    spriteIDB = loadSprite( "testTexture2.tga" );


    parentSpriteID = loadSprite( "parentButton.tga" );
    childSpriteID = loadSprite( "childButton.tga" );

    
    int currentX = 10;
    int currentY = 10;
    
    for( int i=0; i<NUM_DRAWN; i++ ) {
        rgbaColor *c = &( drawColors[i] );
        c->r = 255;//(unsigned char)getRandom( 256 );
        c->g = 255;//(unsigned char)getRandom( 256 );
        c->b = 255;//(unsigned char)getRandom( 256 );
        c->a = (unsigned char)getRandom( 256 );

        deltaFade[i] = -8;
        //deltaFade[i] = 0;
        
        
        drawX[i] = (int)getRandom( 256 );
        drawY[i] = (int)getRandom( 192 );
        /*
        if( i==0 ) {
            drawX[i] = 20;
            drawY[i] = 20;
            //deltaFade[i] = 0;
            //c->a = 255;
            }
        else {
            drawX[i] = 30;
            drawY[i] = 30;
            }
        //deltaFade[i] = 0;
        //c->a = 255;
        */

        deltaX[i] = (int)getRandom( 2 ) - 1;
        deltaY[i] = (int)getRandom( 2 ) - 1;
        
        
        drawX[i] = currentX;
        drawY[i] = currentY;
        
        deltaX[i] = 0;
        deltaY[i] = 0;
        currentX += 3;
        currentY += 3;
        /*
        currentX += 16;
        //currentY += 3;
        if( currentX >= 256 ) {
            currentX = 0;
            currentY += 16;
            }
        */
        }

    char *result = autoSprintf( "test x=%d, s=%s\n", 4123, 
                                "quick brown fox jumped lazy" );
    printOut( result );
    delete [] result;
    
    font8 = new Font( "font8.tga", 0, 6, false );
    font16 = new Font( "font16.tga", 0, 6, false );
    }



void gameFree() {
    delete font8;
    delete font16;

    if( serverAddress != NULL ) {
        delete [] serverAddress;
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
    
    int tx, ty;
    if( getTouch( &tx, &ty ) ) {
        
        if( buttonsVisible ) {
            
            if( isInside( tx, ty, parentButtonX, parentButtonY, 32, 16 ) ) {
                acceptConnection();
                buttonsVisible = false;
                serverAddress = getLocalAddress();
                }
            else if ( isInside( tx, ty, childButtonX,
                                childButtonY, 32, 16 ) ) {
                connectToServer( NULL );
                buttonsVisible = false;
                }
            }
        else {
            
            
            // add new sprite to lower panel
            
            drawX[ indexToTouch ] = tx;
            drawY[ indexToTouch ] = ty;
            
            indexToTouch ++;
            if( indexToTouch >= NUM_DRAWN ) {
                indexToTouch = 0;
                }

            if( checkConnectionStatus() == 1 ) {
                unsigned char message[2];
                message[0] = (unsigned char)tx;
                message[1] = (unsigned char)ty;
                
                sendMessage( message, 2 );
                }
            
            }
        
        
        }
    
    if( checkConnectionStatus() == 1 ) {
        unsigned int numBytes;  
        unsigned char *message = getMessage( &numBytes );
        
        if( message != NULL ) {
            
            if( numBytes == 2 ) {
                
                drawX[ indexToTouch ] = message[0];
                
                drawY[ indexToTouch ] = message[1];
                            
                indexToTouch ++;
                if( indexToTouch >= NUM_DRAWN ) {
                    indexToTouch = 0;
                    }
                }
            
            delete [] message;
            }
        }
    
    }


rgbaColor white = { 255, 255, 255, 255 };




void drawTopScreen() {
    

    for( int i=0; i<NUM_DRAWN; i++ ) {
        drawSprite( spriteID, drawX[i], drawY[i], drawColors[i] );
        startNewSpriteLayer();
        }
    }


void drawBottomScreen() {
    if( buttonsVisible ) {

        drawSprite( parentSpriteID, parentButtonX, parentButtonY, white );
        drawSprite( childSpriteID, childButtonX, childButtonY, white );

        font16->drawString( "Quick brown fox jumped over Lazy Dog", 
                            parentButtonX, 
                            parentButtonY - 20, white, alignLeft );
        
        }
    else {
        
        int netStatus = checkConnectionStatus();
        
        if( netStatus == 0 && serverAddress != NULL ) {
            char *message = autoSprintf( 
                "Waiting for friend to connect to: %s\n", serverAddress );
            
            font8->drawString( message, 
                               parentButtonX, 
                               parentButtonY - 20, white, alignLeft );
            
            delete [] message;
            }
        else if( netStatus == 1 ) {
            
            for( int i=0; i<NUM_DRAWN; i++ ) {
                drawSprite( spriteIDB, drawX[i], drawY[i], drawColors[i] );
                startNewSpriteLayer();
                }
            }
        else if( netStatus == -1 ) {
            font8->drawString( "Connection error", 
                               parentButtonX, 
                               parentButtonY - 20, white, alignLeft );
            }
            
        }
    
    }
