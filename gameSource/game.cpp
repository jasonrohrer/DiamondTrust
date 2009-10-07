

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
extern GameState *flyHomeState;
extern GameState *gameEndState;


Button *doneButton;
Button *nextButton;
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

    doneButton = new Button( font16, translate( "button_done" ), 7, 74 );
    
    nextButton = new Button( font16, translate( "button_next" ), 7, 94 );
    
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
    delete nextButton;
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
    // user hit next, hide sale display
    showSale( false );

    if( getMonthsLeft() > 0 ) {
        decrementMonthsLeft();        
        currentGameState = accumulateDiamondsState;
        }
    else {
        
        // are some units not home?
        char someNotHome = false;
        for( int i=0; i<numPlayerUnits*2; i++ ) {
            int region = getUnitRegion( i );
            if( region != 0 && region != 1 ) {
                someNotHome = true;
                }
            }
        
        
        if( someNotHome ) {
            currentGameState = flyHomeState;
            }
        else {    

            // game over
            currentGameState = gameEndState;
        
            // reveal
            setOpponentMoneyUnknown( false );
            }
        
        }
    
    }



    

static void postBuyTransition() {
    
    if( getMonthsLeft() > 0 ) {
        
        // always do sell transtion, because players earn money even when
        // they have none to sell
        currentGameState = sellDiamondsState;
        }
    else {
        // end of game... selling diamonds never makes sense, so
        // don't let players do it
        postSellTransition();
        }
    
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
    // always hide inspector panel after inspector done moving
    showInspectorPanel( false );
    
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



    
// track whether current state has finished
static char stateDone = false;


// only register a click when stylus/mouse lifted
char touchDownOnLastCheck = false;
int lastTouchX, lastTouchY;


static void goToNextGameState() {
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
    else if( currentGameState == flyHomeState ) {
        currentGameState = depositDiamondsState;
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

    

void gameLoopTick() {
    stepSprites();

    
    
    if( currentGameState->isStateDone() ) {
        if( !stateDone ) {
            // just finished

            if( currentGameState != gameEndState ) {
                // change status sub message to tell player
                // about pushing next button
                statusSubMessage = translate( "subStatus_moveOn" );
                
                // wait for next button before state change
                }
            
            }
        
        stateDone = true;
        }
    else {
        stateDone = false;

        // only step if not done
        currentGameState->stepState();
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
        
        // don't pass clicks to done states
        if( !currentGameState->isStateDone() ) {
            currentGameState->clickState( tx, ty );        
            }
        else if( currentGameState != gameEndState ) {
        
            // next button visible

            if( nextButton->getPressed( tx, ty ) ) {
                goToNextGameState();
                }
            
            }
        
        
        }
    }






void drawTopScreen() {
        
    drawStats();
    

    if( statusMessage != NULL ) {
        
        char *headerString;
        
        if( !currentGameState->isStateDone() &&
            currentGameState != gameEndState ) {
            headerString = translate( "status_next" );
            
            }
        else if( currentGameState->isStateDone() ) {
            // state done
            headerString = translate( "status_finished" );
            }
        else {
            // blank header
            headerString = "";
            }
        
        font16->drawString( headerString, 
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


    if( currentGameState->isStateDone() 
        && currentGameState != gameEndState ) {
    
        nextButton->draw();
        }
    
    }
