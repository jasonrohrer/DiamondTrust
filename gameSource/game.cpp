

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
#include "bidPicker.h"
#include "gameStats.h"
#include "colors.h"




Font *font8;
Font *font16;



//enum gamePhase{ moveUnits };


char *statusMessage = NULL;
char *statusSubMessage = NULL;
//gamePhase currentPhase;


GameState *currentGameState;

extern GameState *connectState;
extern GameState *salaryBribeState;
extern GameState *moveUnitsState;
extern GameState *moveInspectorState;


Button *doneButton;
Button *parentButton;
Button *childButton;



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

    
    printOut( "Loading 8-pixel font\n" );
    font8 = new Font( "font8.tga", 1, 4, false );

    printOut( "Loading 16-pixel font\n" );
    font16 = new Font( "font16.tga", 2, 8, false );
    
    initButton();

    doneButton = new Button( font16, translate( "button_done" ), 7, 84 );
    
    parentButton = new Button( font16, translate( "button_parent" ),
                                                  128 - 32,
                                                  76 - 8 );
    childButton = new Button( font16, translate( "button_child" ),
                                                 128 - 32,
                                                 116 - 8 );

    initMap();
    initUnits();
    initBidPicker();
    initStats();
    
    setPlayerMoney( 0, 18 );
    setPlayerMoney( 1, 18 );    

    currentGameState = connectState;
    currentGameState->enterState();
    }



void gameFree() {
    delete font8;
    delete font16;
    delete doneButton;
    delete parentButton;
    delete childButton;
    
    freeMap();
    freeUnits();
    freeBidPicker();
    freeStats();
    }





// only register a click when stylus/mouse lifted
char touchDownOnLastCheck = false;
int lastTouchX, lastTouchY;


void gameLoopTick() {
    stepSprites();

    currentGameState->stepState();
    
    if( currentGameState->isStateDone() ) {
        
        // state transition
        if( currentGameState == connectState ) {
            currentGameState = salaryBribeState;
            }
        else if( currentGameState == salaryBribeState ) {
            currentGameState = moveUnitsState;
            }
        else if( currentGameState == moveUnitsState ) {
            int briber = getPlayerBribedInspector();
            
            switch( briber ) {
                case -1:
                    currentGameState = moveUnitsState;
                    break;
                case 0:
                case 1:
                    currentGameState = moveInspectorState;
                    break;
                }
            }
        
        
        
        currentGameState->enterState();
        }
        

    int tx, ty;
    if( getTouch( &tx, &ty ) ) {
        touchDownOnLastCheck = true;
        lastTouchX = tx;
        lastTouchY = ty;
        }
    // else not touching
    // was it touching last time?
    else if( touchDownOnLastCheck ) {
        touchDownOnLastCheck = false;
        
        tx = lastTouchX;
        ty = lastTouchY;
        
        // this achieves the equivalent of a "mouse released" event
                
        currentGameState->clickState( tx, ty );        
        }
    }






void drawTopScreen() {
        
    drawStats();
    

    if( statusMessage != NULL ) {
        
        font16->drawString( translate( "status_next" ), 
                            128, 
                            145, white, alignCenter );
        font16->drawString( statusMessage, 
                            128, 
                            160, white, alignCenter );

        if( statusSubMessage != NULL ) {
            font8->drawString( statusSubMessage, 
                               128, 
                               180, white, alignCenter );
            }
        
        }
    
    }


void drawBottomScreen() {
    
    currentGameState->drawState();

    }
