

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

#include "opponent.h"


int drawFrameCounter = false;
int frameCounter = 0;


int satelliteTopSpriteID;
int satelliteBottomSpriteID;
int satelliteBottomHalfOffset;
unsigned char satelliteFade;

int titleSpriteID;
unsigned char titleFade;


// sprites for these have dimensions that are next-largest powers of 2
// (256x128  and 128x64)
int pictureDisplaySpriteID;
int pictureDisplayW = 160;
int pictureDisplayH = 120;
int pictureDisplaySpriteW = 256;
int pictureDisplaySpriteH = 128;


// NOTE:  must be an integer scale factor smaller than display image
// also, WxH must be a multiple of 300 (image sent in 300-char blocks)
int pictureSendSpriteID;
int pictureSendW = 80;
int pictureSendH = 60;
int pictureSendSpriteW = 128;
int pictureSendSpriteH = 64;

// true if fresh data has been set to the sprite for display
char pictureSendSpriteSet = false;



Font *font8;
Font *font16;





char *statusMessage = NULL;
char *statusSubMessage = NULL;


// false for local AI opponent
char networkOpponent;





GameState *currentGameState;

extern GameState *pickGameTypeState;
extern GameState *setAILevelState;
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
// this one can be NULL if not supported
Button *parentServeCloneDownloaButton;
Button *childButton;

Button *aiButton;
Button *wifiButton;

Button *backButton;



static char isInside( int inX, int inY, 
                      int inStartX, int inStartY, int inWidth, int inHeight ) {
    if( inStartX <= inX && inX < inStartX + inWidth 
        &&
        inStartY <= inY && inY < inStartY + inHeight ) {
        return true;
        }
    return false;
    }


#include <stdlib.h>

void gameInit() {

    checkCloneFetch();

    printOut( "Setting random seed to a constant...\n" );
    // note:  we can do this with constant values, because getRandom
    // is called for agent arm waving.... and if we're using the AI,
    // those calls to getRandom will be interleaved in a random way based on 
    // how long the user waits before pressing NEXT
    setRandomSeed( 23423983, 23783294 );
    
    printOut( "Testing getRandom:\n" );
    for( int i=0; i<10; i++ ) {
        printOut( "%d\n", getRandom( 400 ) );
        }
    
    printOut( "Testing getSecondsSinceEpoc: %d\n", getSecondsSinceEpoc() );
    
    printOut( "Setting new random seed based on time\n" );
    setRandomSeed( getSecondsSinceEpoc(), getSecondsSinceEpoc() + 2378923 );
    
    printOut( "Testing getRandom again:\n" );
    for( int i=0; i<10; i++ ) {
        printOut( "%d\n", getRandom( 400 ) );
        }

    // FIXME:  for testing only
    /*
    printOut( "Testing AI...\n" );
    
    printOut( "  ++++++  Init map\n" );
    initMap();

    printOut( "  ++++++  Init units\n" );
    initUnits();

    initOpponent( true );
    unsigned char message[12];
    for( int i=0; i<6; i++ ) {
        message[ i * 2 ] = 0;
        message[ i * 2 + 1] = (unsigned char)( -1 );
        }
    
    sendOpponentMessage( message, 12 );
    
    unsigned int resultLength;
    
    unsigned char *result = NULL;
    while( result == NULL ) {
        stepOpponent();
                result = getOpponentMessage( &resultLength );
        }
    // got move!
    // bail here to get a good profile of just this part of AI
    delete [] result;
    result = NULL;
    
    // actually, play one more move, to test moveUnits
    for( int i=0; i<12; i++ ) {
        // moving nowhere, no bids, no bribes
        message[ i ] = 0;
        }
    

    sendOpponentMessage( message, 9 );
    
    while( result == NULL ) {
        stepOpponent();
        result = getOpponentMessage( &resultLength );
        }
        

    delete [] result;
    printOut( "...done\n" );
    exit( 0 );
    */

    
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


    // satellite top image and camera picture share a sprite set
    // because they are never on the screen at the same time
    int satelliteAndPictureSet = createSpriteSet();
    
    
    printOut( "Loading satellite map\n" );
    satelliteFade = 255;
    int satelliteW, satelliteH;
    rgbaColor *satelliteRGBA = readTGAFile( "angola_satellite.tga",
                                            &satelliteW, &satelliteH );

    if( satelliteRGBA == NULL
        ||
        satelliteW < 256 ) {
        
        printOut( "Reading satellite map file failed.\n" );
        return;
        }
    satelliteBottomHalfOffset = 128;
    
    satelliteTopSpriteID = 
        addSprite( satelliteRGBA, satelliteW, satelliteBottomHalfOffset,
                   satelliteAndPictureSet );
    
    rgbaColor *bottomHalfPointer = 
        &( satelliteRGBA[ satelliteBottomHalfOffset * satelliteW ] );
    
    satelliteBottomSpriteID = 
        addSprite( bottomHalfPointer, satelliteW, 
                   satelliteH - satelliteBottomHalfOffset );
    // FIXME
    //satelliteTopSpriteID = satelliteBottomSpriteID;
    
    delete [] satelliteRGBA;


    printOut( "Loading title image\n" );
    titleFade = 255;

    int titleW, titleH;
    rgbaColor *titleRGBA = readTGAFile( "titleBlack16.tga",
                                        &titleW, &titleH );

    applyCornerTransparency( titleRGBA, titleW * titleH );

    if( titleRGBA == NULL ) {
        
        printOut( "Reading title file failed.\n" );
        return;
        }

    /*
    // split into 3 parts to save texture memory
    int bandHeights[3] = { 64, 16, 8 };
    int nextBandOffset = 0;
    for( int b=0; b<3; b++ ) {
        rgbaColor *subPointer = 
            &( titleRGBA[ nextBandOffset * titleW ] );
    
        titleSpriteID[b] = 
            addSprite( subPointer, titleW, bandHeights[b] );
        
        nextBandOffset += bandHeights[b];
        }
    */
    
    titleSpriteID = 
            addSprite( titleRGBA, titleW, titleH );

    delete [] titleRGBA;
    


    printOut( "Constructing camera picture sprites\n" );

    // grayscale palette
    unsigned short *picturePalette = new unsigned short[ 256 ];
    for( int p=0; p<256; p++ ) {

        // 5-bit gray
        int grayLevel = p >> 3;
        
        picturePalette[p] = (unsigned short)( 
            1  << 15 |
            grayLevel << 10 |
            grayLevel << 5 |
            grayLevel );
        }
    
    

    // dummy data for now
    unsigned char *data = 
        new unsigned char[ pictureDisplaySpriteW * pictureDisplaySpriteH ];
    
    memset( data, 0, 
            (unsigned int)( pictureDisplaySpriteW * pictureDisplaySpriteH ) );

    // share set with top half of satellite
    pictureDisplaySpriteID = 
        addSprite256( data, 
                      pictureDisplaySpriteW, pictureDisplaySpriteH,
                      picturePalette,
                      false, 
                      satelliteAndPictureSet );
    delete [] data;
    
    data = 
        new unsigned char[ pictureSendSpriteW * pictureSendSpriteH ];
    
    memset( data, 0, 
            (unsigned int)( pictureSendSpriteW * pictureSendSpriteH ) );

    pictureSendSpriteID = 
        addSprite256( data, 
                      pictureSendSpriteW, pictureSendSpriteH,
                      picturePalette );
    delete [] data;
    delete [] picturePalette;
    



    printOut( "Loading 8-pixel font\n" );
    font8 = new Font( "font8.tga", 1, 4, false );

    printOut( "Loading 16-pixel font\n" );
    font16 = new Font( "font16_sans.tga", 2, 6, false );
    
    initButton();

    doneButton = new Button( font16, translate( "button_done" ), 38, 87 );
    
    nextButton = new Button( font16, translate( "button_next" ), 38, 111 );
    
    

    if( isCloneBootPossible() ) {
        // two options for hosting
        
        parentButton = new Button( 
            font16, 
            translate( "button_parent_muti_card" ),
            128,
            66 );
        parentServeCloneDownloaButton = new Button( 
            font16, 
            translate( "button_parent_clone" ),
            128,
            106 );
        }
    else {
        // just a straight hosting button
    
        parentServeCloneDownloaButton = NULL;
        
        parentButton = new Button( font16, translate( "button_parent" ),
                                   128,
                                   106 );
        }
    
    childButton = new Button( font16, translate( "button_child" ),
                                                 128,
                                                 146 );


    aiButton = new Button( font16, translate( "button_ai" ),
                           128,
                           148 );

    wifiButton = new Button( font16, translate( "button_wifi" ),
                             128,
                             178 );


    printOut( "  ++++++  Init map\n" );
    initMap();

    printOut( "  ++++++  Init units\n" );
    initUnits();

    printOut( "  ++++++  Init bid picker\n" );
    initBidPicker();

    printOut( "  ++++++  Init game stats\n" );
    initStats();

    printOut( "  ++++++  Init flying diamonds\n" );
    initFlyingDiamonds();

    printOut( "  ++++++  Init sale picker\n" );
    initSalePicker();
    

    setPlayerMoney( 0, 18 );
    setPlayerMoney( 1, 18 );    

    setPlayerDiamonds( 0, 0 );
    setPlayerDiamonds( 1, 0 );    

    // FIXME:  testing
    //setPlayerDiamonds( 0, 10 );
    //setPlayerDiamonds( 1, 10 );    

    // actually, it makes sense for players to start the game with a few
    // diamonds (so that the SellDiamonds phase has an interesting decision
    // right off the bat).  2 each allows full exploration of payoff matrix.
    setPlayerDiamonds( 0, 2 );
    setPlayerDiamonds( 1, 2 );    


    // show opponent's money at start of game
    setOpponentMoneyUnknown( false );
    
    setMonthsLeft( 8 );
    

    currentGameState = pickGameTypeState;
    //currentGameState = connectState;
    //currentGameState = sellDiamondsState;
    currentGameState->enterState();
    }



void gameFree() {
    delete font8;
    delete font16;
    delete doneButton;
    delete nextButton;

    delete aiButton;
    delete wifiButton;
    
    if( parentServeCloneDownloaButton != NULL ) {
        delete parentServeCloneDownloaButton;
        parentServeCloneDownloaButton = NULL;
        }

    delete parentButton;
    delete childButton;
    
    freeMap();
    freeUnits();
    freeBidPicker();
    freeStats();
    freeFlyingDiamonds();
    freeSalePicker();

    freeOpponent();
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
    if( currentGameState == pickGameTypeState ) {

        if( networkOpponent ) {    
            currentGameState = connectState;
            }
        else {
            // set AI level here
            currentGameState = setAILevelState;
            }
        }
    else if( currentGameState == setAILevelState ) {
        // start game
        currentGameState = accumulateDiamondsState;
        }
    else if( currentGameState == connectState ) {
        currentGameState = accumulateDiamondsState;
        // FIXME  for testing
        //currentGameState = sellDiamondsState;
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
    stepOpponent();
    
    
    
    if( currentGameState->isStateDone() ) {
        if( !stateDone ) {
            // just finished

            if( currentGameState != gameEndState ) {

                if( currentGameState->needsNextButton() ) {
                    
                    // change status sub message to tell player
                    // about pushing next button
                    statusSubMessage = translate( "subStatus_moveOn" );
                    
                    // wait for next button before state change
                    }
                else {
                    // move on instantly
                    goToNextGameState();
                    // reset the frame counter after each NEXT
                    frameCounter = 0;
                    }
                
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
                // reset the frame counter after each NEXT
                frameCounter = 0;
                }
            
            }
        
        
        }
    }






void drawTopScreen() {
    
    drawStats();
    

    if( statusMessage != NULL ) {
        
        char *headerString;
        
        // Only show "Finished:" if NEXT button displayed
        // avoids a flicker when auto-advancing to the next state
        if( ( !currentGameState->isStateDone() || 
              !currentGameState->needsNextButton() ) &&
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
                            148, white, alignCenter );
        
        font16->drawString( statusMessage, 
                            128, 
                            163, white, alignCenter );

        if( statusSubMessage != NULL ) {
            font8->drawString( statusSubMessage, 
                               128, 
                               180, white, alignCenter );
            }
        
        }

    if( drawFrameCounter ) {
        
        startNewSpriteLayer();
    
        char *countString = autoSprintf( "%d", frameCounter / 30 );
        
        font16->drawString( countString, 128, 130, red, alignCenter );
        
        delete [] countString;
        }
    
    if( currentGameState != pickGameTypeState ) {
        // start counter as soon as player touches first button
        frameCounter++;
        }
    
    }


void drawBottomScreen() {
    
    currentGameState->drawState();


    if( currentGameState->isStateDone() 
        && currentGameState->needsNextButton()
        && currentGameState != gameEndState ) {
    
        nextButton->draw();
        }
    
    }
