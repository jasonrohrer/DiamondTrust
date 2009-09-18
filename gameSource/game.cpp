

#include "platform.h"
#include "common.h"
#include "tga.h"
#include "Font.h"
#include "Button.h"
#include "map.h"
#include "units.h"
#include "sprite.h"
#include "minorGems/util/stringUtils.h"
#include "TranslationManager.h"
#include "GameState.h"


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

int testDirectSpriteID = -1;
int test250SpriteID = -1;


int parentButtonX = 10;
int parentButtonY = 150;
int childButtonX = 10;
int childButtonY = 170;


char buttonsVisible = true;

char *serverAddress = NULL;


Font *font8;
Font *font16;



//enum gamePhase{ moveUnits };


char *statusMessage = NULL;
//gamePhase currentPhase;


GameState *currentGameState;

extern GameState *moveUnitsState;


Button *doneButton;



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
    
    int dataSize;
    unsigned char *languageData = readFile( "English.txt", &dataSize );
    
    if( languageData != NULL ) {
        char *textData = new char[ dataSize + 1 ];
        memcpy( textData, languageData, (unsigned int)dataSize );
        textData[ dataSize ] = '\0';
        
        delete [] languageData;

        TranslationManager::setLanguageData( textData );
        delete [] textData;
        }
    
    printOut( "Loading test sprites\n" );
    
    spriteID = loadSprite( "testTexture.tga", true );
    spriteIDB = loadSprite( "testTexture2.tga" );
    
    parentSpriteID = loadSprite( "parentButton.tga" );
    childSpriteID = loadSprite( "childButton.tga" );

    

    testDirectSpriteID = loadSprite( "testDirectColor.tga" );
    test250SpriteID = loadSprite( "test250Color.tga" );
    

    
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
    printOut( "\n" );
    
    
    printOut( "Loading 8-pixel font\n" );
    font8 = new Font( "font8.tga", 1, 6, false );

    printOut( "Loading 16-pixel font\n" );
    font16 = new Font( "font16.tga", 2, 8, false );
    
    initButton();

    doneButton = new Button( font16, translate( "button_done" ), 7, 84 );
    

    initMap();
    initUnits();
    //setRegionSelectable( 2, true );
    //setRegionSelectable( 4, true );
    

    currentGameState = moveUnitsState;
    currentGameState->enterState();
    }



void gameFree() {
    delete font8;
    delete font16;
    delete doneButton;
    
    if( serverAddress != NULL ) {
        delete [] serverAddress;
        }

    freeMap();
    freeUnits();
    }



int fade = 255;
int dFade = -8;
int x = 0;
int dX = 1;

void gameLoopTick() {
    stepSprites();

    currentGameState->stepState();
    

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
        
        currentGameState->clickState( tx, ty );
        
        /*
        int regionHit = getChosenRegion( tx, ty );
        
        if( regionHit >= 0 ) {
            setRegionSelectable( regionHit, true );
            }
        */
        
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
rgbaColor transBlack = { 0, 0, 0, 128 };
rgbaColor black = { 0, 0, 0, 255 };




void drawTopScreen() {
    
    drawSprite( testDirectSpriteID, 100, 50, white );
    drawSprite( test250SpriteID, 100, 120, white );
    

    for( int i=0; i<1; i++ ) { //NUM_DRAWN; i++ ) {
        drawSprite( spriteID, drawX[i], drawY[i], drawColors[i] );
        startNewSpriteLayer();
        }

    font16->drawString( translate( "status_next" ), 
                        128, 
                        160, white, alignCenter );
    font16->drawString( statusMessage, 
                        128, 
                        175, white, alignCenter );
    }


void drawBottomScreen() {
    
    currentGameState->drawState();
    

    if( buttonsVisible ) {

        drawSprite( parentSpriteID, parentButtonX, parentButtonY, white );
        drawSprite( childSpriteID, childButtonX, childButtonY, white );

        
        font16->drawString( "Quick brown fox jumped over Lazy Dog", 
                            parentButtonX + 2, 
                            parentButtonY - 20 + 1, black, alignLeft );

        

        startNewSpriteLayer();
        
        font16->drawString( "Quick brown fox jumped over Lazy Dog", 
                            parentButtonX, 
                            parentButtonY - 20, white, alignLeft );
        
        }
    else {
        
        int netStatus = checkConnectionStatus();
        
        if( netStatus == 0 && serverAddress != NULL ) {
            char *message = autoSprintf( 
                "Waiting for connection on: %s\n", serverAddress );
            
            font16->drawString( message, 
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
