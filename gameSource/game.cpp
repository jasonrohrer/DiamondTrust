

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
#include "ai/ai.h"
#include "opponent.h"
#include "music.h"
#include "DotMatrixRGBAFilter.h"
#include "greenBarPaper.h"



// set to false when gameInit done
// allows gameLoopTick and drawTopScreen/drawBottomScreen to execute special
// behavior when everything hasn't been loaded yet
static char stillLoading = true;

#define PROGRESS_LENGTH 58
static char loadingProgress[ PROGRESS_LENGTH  + 1 ];

#include "loading.h"


// show extra info about song that's playing, along with a "next act" button,
// on the title screen, for debugging and testing
char allowManualSongActSwitching = true;



int drawFrameCounter = false;
int frameCounter = 0;


int satelliteTopSpriteID;
int satelliteBottomSpriteID;
int satelliteBottomHalfOffset;
unsigned char satelliteFade;

int titleSpriteID;
unsigned char titleFade;

int wirelessOnSpriteID;


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



int globalSoundVolume = 0;



Font *font8 = NULL;
Font *font16 = NULL;



int satPhoneSpriteID = -1;

int signalSpriteID[4];



char *statusMessage = NULL;
char *statusSubMessage = NULL;


// false if connected to a game hosted elsewhere
// (true even if this is an AI game)
char isHost = true;


// false for local AI opponent
char networkOpponent;


char isWaitingOnOpponent = false;




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
extern GameState *connectionBrokenState;


Button *doneButton;
Button *nextButton;
Button *parentButton;
// this one can be NULL if not supported
Button *parentServeCloneDownloaButton;
Button *childButton;

// for testing
Button *nextSongActButton;
Button *songRerollButton;


Button *aiButton;
Button *wifiButton;

Button *backButton;

Button *playAgainButton;



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

    for( int i=0; i<PROGRESS_LENGTH; i++ ) {
        loadingProgress[i] = '-';
        }
    loadingProgress[ PROGRESS_LENGTH ] = '\0';
    
    


    checkCloneFetch();

    printOut( "Setting random seed to a constant...\n" );
    // note:  we can do this with constant values, because getRandom
    // is called for agent arm waving.... and if we're using the AI,
    // those calls to getRandom will be interleaved in a random way based on 
    // how long the user waits before pressing NEXT
    setRandomSeed( 23423983, 23783294 );
    
    printOut( "Testing GLOBAL getRandom (const seed):\n" );
    for( int i=0; i<10; i++ ) {
        printOut( "%d\n", getRandom( 400 ) );
        }
    
    // test customRand to make sure it doesn't interfere with global one
    setRandomSeed( 23423983, 23783294 );
    
    randState rState = startCustomRand( 23423 );
    
    printOut( "Testing CUSTOM getRandom:\n" );
    for( int i=0; i<10; i++ ) {
        printOut( "%d\n", getRandom( &rState, 400 ) );
        }

    printOut( "Testing GLOBAL getRandom again (const seed):\n" );
    for( int i=0; i<10; i++ ) {
        printOut( "%d\n", getRandom( 400 ) );
        }




    printOut( "Testing getSecondsSinceEpoc: %d\n", getSecondsSinceEpoc() );
    
    printOut( "Setting new random seed based on time\n" );
    setRandomSeed( getSecondsSinceEpoc(), getSecondsSinceEpoc() + 2378923 );
    
    printOut( "Testing GLOBAL getRandom again (with time seed):\n" );
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

    printOut( "Loading 16-pixel font\n" );
    DotMatrixRGBAFilter fontFilter;
    font16 = new Font( "font16_jcr.tga", 2, 6, false, &fontFilter );

    printOut( "  ++++++  Init green bar paper\n" );
    initGreenBarPaper();


    // we can call this now that we have at least the big font
    // AND the greenbar to go behind it
    updateLoadingProgress();

    
    printOut( "Loading sat phone and signal meter\n" );
    satPhoneSpriteID = loadSprite( "satPhone.tga", true );

    for( int i=0; i<4; i++ ) {
        char *fileName = autoSprintf( "signal%d.tga", i );
        
        signalSpriteID[i] = loadSprite( fileName, false );
        
        delete [] fileName;
        }
    
    printOut( "Loading 8-pixel font\n" );
    font8 = new Font( "font8.tga", 1, 4, false );


    // call agin with both fonts (first progress bar step)
    updateLoadingProgress();
    

    


    // satellite top image and camera picture share a sprite set
    // because they are never on the screen at the same time
    int satelliteAndPictureSet = createSpriteSet();
    
    
    printOut( "Loading satellite map\n" );
    satelliteFade = 255;
    int satelliteW, satelliteH;
    rgbaColor *satelliteRGBA = readTGAFile( "angola_satellite.tga",
                                            &satelliteW, &satelliteH );

    updateLoadingProgress();

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

    updateLoadingProgress();

    rgbaColor *bottomHalfPointer = 
        &( satelliteRGBA[ satelliteBottomHalfOffset * satelliteW ] );
    
    satelliteBottomSpriteID = 
        addSprite( bottomHalfPointer, satelliteW, 
                   satelliteH - satelliteBottomHalfOffset );

    updateLoadingProgress();

    // FIXME
    //satelliteTopSpriteID = satelliteBottomSpriteID;
    
    delete [] satelliteRGBA;


    printOut( "Loading title image\n" );
    titleFade = 255;

    int titleW, titleH;
    rgbaColor *titleRGBA = readTGAFile( "titleBlack16.tga",
                                        &titleW, &titleH );

    applyCornerTransparency( titleRGBA, titleW * titleH );

    updateLoadingProgress();


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
    

    wirelessOnSpriteID = loadSprite( "wirelessOn.tga", false );


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
    



    
    initButton();
    updateLoadingProgress();

    doneButton = new Button( font16, translate( "button_done" ), 38, 111 );
    
    nextButton = new Button( font16, translate( "button_next" ), 38, 87 );

    backButton = new Button( font16, translate( "button_back" ), 38, 174 );

    playAgainButton = 
        new Button( font16, translate( "button_play_again" ), 69, 111 );
    
    

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


    nextSongActButton = new Button( font16, translate( "button_nextSongAct" ),
                                    38, 87 );

    songRerollButton = new Button( font16, translate( "button_songReroll" ),
                                    128, 87 );





    aiButton = new Button( font16, translate( "button_ai" ),
                           128,
                           148 );

    wifiButton = new Button( font16, translate( "button_wifi" ),
                             128,
                             178 );


    printOut( "  ++++++  Init map\n" );
    initMap();
    updateLoadingProgress();

    printOut( "  ++++++  Init units\n" );
    initUnits();
    updateLoadingProgress();

    printOut( "  ++++++  Init bid picker\n" );
    initBidPicker();
    updateLoadingProgress();

    printOut( "  ++++++  Init game stats\n" );
    initStats();
    updateLoadingProgress();

    printOut( "  ++++++  Init flying diamonds\n" );
    initFlyingDiamonds();
    updateLoadingProgress();

    printOut( "  ++++++  Init sale picker\n" );
    initSalePicker();
    updateLoadingProgress();

    


    resetStats();
    


    initMusic();
    

    
    if( ! isAutoconnecting() ) {
        currentGameState = pickGameTypeState;
        }
    else {
        titleFade = 0;
        currentGameState = connectState;
        
        // we skip the pickGameTypeState, so set this here
        networkOpponent = true;
        }
    
    //currentGameState = connectState;
    //currentGameState = sellDiamondsState;
    currentGameState->enterStateCall();

    stillLoading = false;
    }



void gameFree() {
    delete font8;
    delete font16;
    delete doneButton;
    delete nextButton;
    delete backButton;
    delete playAgainButton;
    
    delete nextSongActButton;
    delete songRerollButton;
    
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
    freeGreenBarPaper();
    
    freeOpponent();

    freeMusic();
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



static void resetToPlayAgain() {
    resetStats();
    resetMapDiamondCounts();
    resetUnits();
    // all units are home and empty
    
    // leave inspector alone

    resetAI();
    }


    
// track whether current state has finished
static char stateDone = false;


// only register a click when stylus/mouse lifted
char touchDownOnLastCheck = false;
int lastTouchX, lastTouchY;


static void goToNextGameState() {
    // state transition

    // special case if connection ever broken:
    // go right to connectionBrokenState from ANY state
    if( currentGameState->isConnectionBroken() ) {
        currentGameState = connectionBrokenState;
        }
    else if( currentGameState == pickGameTypeState ) {

        if( networkOpponent ) {    
            currentGameState = connectState;
            }
        else {
            // set AI level here
            
            // we're "hosting" a game vs. the AI
            isHost = true;
            
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
    else if( currentGameState == gameEndState ) {

        if( gameEndState->getParameter() ) {
            // play again!
            currentGameState = accumulateDiamondsState;
            }
        else {
            // back to menu
            currentGameState = pickGameTypeState;
            }

        resetToPlayAgain();
        }
    else if( currentGameState == connectionBrokenState ) {
        // back to menu
        currentGameState = pickGameTypeState;
        resetToPlayAgain();
        }

                
    currentGameState->enterStateCall();
    }



static void goToPreviousGameState() {
    // inform state that it has been cancelled
    currentGameState->backOutState();
    

    

    // state BACK transition


    if( currentGameState == setAILevelState ||
        currentGameState == connectState ||
        currentGameState == gameEndState ) {

        // back to title
        resetToPlayAgain();
        currentGameState = pickGameTypeState;
        }
                
    currentGameState->enterStateCall();
    }

    

void gameLoopTick() {
    if( stillLoading ) {
        
        // no tick
        return;
        }
    

    if( globalSoundVolume == 0 ) {
        
        // set up pan for all channels
        
        // every-other channel is either leftish or rightish
        // we don't know how many total channels we're going to have
        int panFlip = -1;

        for( int c=0; c<8; c++ ) {
            int pan = 64 + panFlip * 16;

            panFlip *= -1;
            
            printOut( "Setting channel %d pan to %d\n", c, pan );
            
            setSoundChannelPan( c, pan  );
            }
        
        }
    if( globalSoundVolume < 127 ) {
        globalSoundVolume++;
        
        for( int c=0; c<8; c++ ) {
            setSoundChannelVolume( c, globalSoundVolume );
            }
        }
    


    stepSprites();
    stepOpponent();
    
    
    
    if( currentGameState->isStateDone() ) {
        // state never ends while we're still waiting for a response!
        isWaitingOnOpponent = false;
        
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
            else {
                // play again!

                // move on instantly
                goToNextGameState();

                // reset the frame counter after each NEXT
                frameCounter = 0;
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
            
            char touchEaten = false;
            
            if( currentGameState->canStateBeBackedOut() ) {
                
                if( backButton->getPressed( tx, ty ) ) {
                    touchEaten = true;
                    
                    goToPreviousGameState();
                    // reset the frame counter after each BACK
                    frameCounter = 0;
                    }
                }
            
            if( ! touchEaten ) {
                // pass through to state
                currentGameState->clickState( tx, ty );
                }
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





unsigned int frameBatchStartTime = 0;

unsigned int frameBatchCount = 0;



void drawTopScreen() {
      
    if( frameBatchStartTime == 0 ) {
        frameBatchStartTime = getSystemMilliseconds();
        frameBatchCount = 0;
        }
    
    
    frameBatchCount++;
    

    if( frameBatchCount == 100 ) {
        unsigned int endTime = getSystemMilliseconds();
        
        printOut( "Frame rate (per 10 seconds):  %d\n", 
                  10 * 1000 * 100 / ( endTime - 
                                 frameBatchStartTime ) );
        // next batch 
        frameBatchStartTime = endTime;
        frameBatchCount = 0;
        }
    

    if( satPhoneSpriteID != -1 ) {
        int sigStrength = getSignalStrength();
        
        if( sigStrength >= 0 && sigStrength <= 3 ) {
            
            int phoneX = 230;
            int phoneY = 146;

            drawSprite( satPhoneSpriteID, 
                        phoneX, phoneY, white );
            
            drawSprite( signalSpriteID[ sigStrength ], 
                        phoneX + 8, phoneY + 6, white );
            }
        }
    
    
    if( stillLoading ) {

        if( font16 != NULL ) {
            
            printOut( "Drawing LOADING message on top screen\n" );
            
            drawGreenBarPaper( 148, 191 );
            
            startNewSpriteLayer();

            font16->drawString( translate( "status_loading" ), 
                                128, 
                                166, white, alignCenter );
            }
        else {
            printOut( "Large font not loaded, so not drawing "
                      "LOADING message\n" );
            }
        
 
        if( font8 != NULL ) {
            printOut( "Progress: %s\n", loadingProgress );

            font8->drawString( loadingProgress, 
                               12, 
                               183, black, alignLeft );
            }
        else {
            printOut( "Small font not loaded, so not drawing "
                      "progress bar\n" );
            }

        // draw nothing else, because it's not loaded
        return;
        }
    

    drawStats();
    

    if( statusMessage != NULL ) {
        
        char *headerString;
        
        // Only show "Finished:" if NEXT button displayed
        // avoids a flicker when auto-advancing to the next state
        if( ( !currentGameState->isStateDone() || 
              !currentGameState->needsNextButton() ) ) {
            
            // hide Next label for game end state
            if( currentGameState != gameEndState && 
                currentGameState != connectionBrokenState ) {
                
                headerString = translate( "status_next" );
                }
            else {
                headerString = "";
                }
            }
        else if( currentGameState->isStateDone() ) {
            // state done
            headerString = translate( "status_finished" );
            }
        else {
            // blank header
            headerString = "";
            }
        

        drawGreenBarPaper( 148, 191 );

        startNewSpriteLayer();

        font16->drawString( headerString, 
                            128, 
                            151, white, alignCenter );
        
        font16->drawString( statusMessage, 
                            128, 
                            166, white, alignCenter );

        if( statusSubMessage != NULL ) {
            font8->drawString( statusSubMessage, 
                               128, 
                               183, black, alignCenter );
            }

        if( isWaitingOnOpponent && ! networkOpponent ) {
            
            int progress = getAIProgress( PROGRESS_LENGTH );

            if( progress > 0 ) {
                // After AI finishes, we wait until state is done
                // before isWaitingOnOpponent resets
                // don't show an empty bar after AI finishes!


                // show a progress bar for the AI
                
                char progressString[ PROGRESS_LENGTH + 1 ];
                
                progressString[ PROGRESS_LENGTH ] = '\0';
                

                // looks the same as the loading bar
                memset( progressString, '-', PROGRESS_LENGTH + 1 );
                
                memset( progressString, ')', (unsigned int)progress );
                
                
                font8->drawString( progressString, 
                                   12, 
                                   120, white, alignLeft );
                }
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

    if( allowManualSongActSwitching ) {
        char *stateString = getLastMusicState();
        
        if( stateString == NULL ) {
            stateString = "";
            }
        
        char *actString = autoSprintf( "Act: %d    Seed string: '%s' ", 
                                       getSongAct(), stateString );
        
        font8->drawString( actString, 5, 35, white, alignLeft );

        delete [] actString;

        int numPartStrings;
        
        char **partStrings = getTrackInfoStrings( &numPartStrings );
        
        for( int i=0; i<numPartStrings; i++ ) {
            
            rgbaColor fontColor = white;
            
            if( i % 2 != 0 ) {
                fontColor = gray;
                }

            char found;
            
            char *shortPartString = replaceOnce( 
                partStrings[i], 
                "music/", "", &found );

            font8->drawString( shortPartString, 
                                5, 49 + 12 * i, fontColor, alignLeft );
            
            delete [] shortPartString;
            delete [] partStrings[i];
            }

        delete [] partStrings;
        }
    
    }


void drawBottomScreen() {
    
    if( stillLoading ) {
        // draw nothing here
        return;
        }
    

    currentGameState->drawState();


    if( currentGameState->isStateDone() 
        && currentGameState->needsNextButton()
        && currentGameState != gameEndState ) {
    
        nextButton->draw();
        }
    else if( ! currentGameState->isStateDone() &&
             currentGameState->canStateBeBackedOut() ) {
        
        backButton->draw();
        }
    }



void updateLoadingProgress() {

    runGameLoopOnce();
    runGameLoopOnce();
    
    // add another tick to progress
    for( int i=0; i<PROGRESS_LENGTH; i++ ) {
        if( loadingProgress[i] == '-' ) {
            loadingProgress[i] = ')';
            printOut( "Setting new tick at %d\n", i );
            break;
            }
        }
    }
