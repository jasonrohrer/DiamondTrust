

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
#include "flyingDiamonds.h"
#include "salePicker.h"




Font *font8;
Font *font16;



//enum gamePhase{ moveUnits };


char *statusMessage = NULL;
char *statusSubMessage = NULL;
//gamePhase currentPhase;


GameState *currentGameState;

extern GameState *connectState;
extern GameState *accumulateDiamondsState;
extern GameState *salaryBribeState;
extern GameState *moveUnitsState;
extern GameState *depositDiamondsState;
extern GameState *confiscateState;
extern GameState *moveInspectorState;
extern GameState *buyDiamondsState;
extern GameState *sellDiamondsState;
extern GameState *gameEndState;


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
                                                  128 - 64,
                                                  76 - 8 );
    childButton = new Button( font16, translate( "button_child" ),
                                                 128 - 64,
                                                 116 - 8 );

    initMap();
    initUnits();
    initBidPicker();
    initStats();
    initFlyingDiamonds();
    initSalePicker();
    
    setPlayerMoney( 0, 18 );
    setPlayerMoney( 1, 18 );    

    // show opponent's money at start of game
    setOpponentMoneyUnknown( false );
    
    setMonthsLeft( 12 );
    

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
    freeFlyingDiamonds();
    freeSalePicker();
    }


static void postAccumulateTransition() {
    if( isAnyUnitPayable() ) {
        currentGameState = salaryBribeState;
        }
    else {
        // skip salary
        currentGameState = moveUnitsState;
        }
    }



static void postSellTransition() {

    if( getMonthsLeft() > 0 ) {
        decrementMonthsLeft();        
        currentGameState = accumulateDiamondsState;
        }
    else {
        // game over
        currentGameState = gameEndState;
        
        // reveal
        setOpponentMoneyUnknown( false );
        }
    
    }



    

static void postBuyTransition() {
    printOut( "Transition after buy\n" );
    

    // always do sell transtion, because players earn money even when
    // they have none to sell
    currentGameState = sellDiamondsState;
    }



static void postConfiscateTransition() {
    if( isAnyUnitBuyingDiamonds() ) {
        currentGameState = buyDiamondsState;
        }
    else {
        // skip buying
        postBuyTransition();
        }
    }

        

static void postMoveInspectorTransition() {
    if( isAnyConfiscationNeeded() ) {
        currentGameState = confiscateState;
        }
    else {
        // skip confiscation
        postConfiscateTransition();
        }
    }



static void postDepositeTransition() {
    int briber = getPlayerBribedInspector();
            
    switch( briber ) {
        case -1:
            // skip moving inspector
            postMoveInspectorTransition();
            break;
        case 0:
        case 1:
            currentGameState = moveInspectorState;
            break;
        }
    }


static void postMoveUnitsTransition() {
    if( isAnyUnitDepositingDiamonds() ) {
        currentGameState = depositDiamondsState;
        }
    else {
        // skip deposit
        postDepositeTransition();
        }
    }



    
// force a pause between all state transitions to give player a chance
// to read screen
static int stateDoneTicks = 0;
static int minStateDoneTicks = 60;



// only register a click when stylus/mouse lifted
char touchDownOnLastCheck = false;
int lastTouchX, lastTouchY;


void gameLoopTick() {
    stepSprites();

    
    
    if( currentGameState->isStateDone() ) {
        stateDoneTicks ++;
        }
    else {
        stateDoneTicks = 0;

        // only step if not done
        currentGameState->stepState();
        }
    
    if( stateDoneTicks > minStateDoneTicks ) {
        // enough ticks have passed since last state ended
        
        
        // state transition
        if( currentGameState == connectState ) {
            currentGameState = accumulateDiamondsState;
            }
        else if( currentGameState == accumulateDiamondsState ) {
            postAccumulateTransition();
            }
        else if( currentGameState == salaryBribeState ) {
            currentGameState = moveUnitsState;
            }
        else if( currentGameState == moveUnitsState ) {
            postMoveUnitsTransition();
            }
        else if( currentGameState == depositDiamondsState ) {
            postDepositeTransition();
            }
        else if( currentGameState == moveInspectorState ) {
            postMoveInspectorTransition();
            }
        else if( currentGameState == confiscateState ) {
            postConfiscateTransition();
            }
        else if( currentGameState == buyDiamondsState ) {
            postBuyTransition();
            }
        else if( currentGameState == sellDiamondsState ) {
            // beginning of next round
            postSellTransition();
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
