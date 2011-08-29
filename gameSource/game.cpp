

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
#include "BluePenRGBAFilter.h"
#include "help.h"
#include "watch.h"




// set to false when gameInit done
// allows gameLoopTick and drawTopScreen/drawBottomScreen to execute special
// behavior when everything hasn't been loaded yet
static char stillLoading = true;

#define PROGRESS_LENGTH 55
static char loadingProgress[ PROGRESS_LENGTH  + 1 ];
static int currentLoadingProgress = 0;




// starts off the bottom of the screen as credits scroll up
static int bottomGreenbarSheetTop = 192;


// scrolls up, redrawing screen as needed, to show next credit
static void scrollCreditSheetUp();


// for fading in a black rectangle over game map as help scrolls up
static int helpFadeStepsMax = 50;



#include "loading.h"


// show extra info about song that's playing, along with a "next act" button,
// on the title screen, for debugging and testing
char allowManualSongActSwitching = false;



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



int aiLevelSheetSpriteID;


int globalSoundVolume = 0;
// start off with volume all the way down, and stuck there
// set to false to force a gradual fade-out
// set to true to force a gradual fade-in
int globalVolumeRise = false;

char soundPansSet = false;



Font *font8 = NULL;
Font *font16 = NULL;
Font *font16Hand = NULL;


int greenBarLeftMargin = 18;



int signalSpriteID[4];



const char *statusMessage = NULL;
const char *statusSubMessage = NULL;


// false if connected to a game hosted elsewhere
// (true even if this is an AI game)
char isHost = true;


// false for local AI opponent
char networkOpponent;


char isWaitingOnOpponent = false;


// make sure we don't show progress bar for only a brief flicker
char isAIProgressShowing = false;


int stepsSinceLidClosedMusicChange = 0;

int stepsToWaitBeforeLidClosedMusicChange = 0;




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
Button *parentServeCloneDownloadButton;
Button *childButton;

Button *helpButton;


// for testing
Button *nextSongActButton;
Button *songRerollButton;
Button *switchSongButton;


Button *aiButton;
Button *wifiButton;

Button *backButton;

Button *playAgainButton;



static void setMusicBasedOnGameState() {
    char *posString = getUnitPositionString();
    
    char *stateString = autoSprintf( "State%d, %s",
                                     currentGameState->mStateNumber,
                                     posString );
    setMusicState( stateString );
    
    delete [] posString;
    delete [] stateString;    
    }





#include <stdlib.h>

void gameInit() {

    for( int i=0; i<PROGRESS_LENGTH; i++ ) {
        loadingProgress[i] = ' ';
        }
    loadingProgress[ PROGRESS_LENGTH ] = '\0';
    currentLoadingProgress = 0;
    


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

    // seed with a constant for testing audio
    // setRandomSeed( 23423983, 23783294 );


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
    font16 = new Font( "font16_jcr.tga", 2, 5, false, &fontFilter );

    printOut( "  ++++++  Init green bar paper\n" );
    initGreenBarPaper();


    // we can call this now that we have at least the big font
    // AND the greenbar to go behind it
    updateLoadingProgress();

    
    printOut( "Loading signal meter\n" );

    for( int i=0; i<4; i++ ) {
        char *fileName = autoSprintf( "signal%d.tga", i );
        
        signalSpriteID[i] = loadSprite( fileName, false );
        
        delete [] fileName;
        }
    
    printOut( "Loading 8-pixel font\n" );
    font8 = new Font( "font8.tga", 1, 4, false );
    font8->setRaiseDollarSign( true );
    

    // call agin with both fonts (first progress bar step)
    updateLoadingProgress();
    

    scrollCreditSheetUp();

    BluePenRGBAFilter penFontFilter;
    font16Hand = 
        new Font( "font16_handwriting2.tga", 2, 6, false, &penFontFilter );
    


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

    aiLevelSheetSpriteID = 
        loadSprite( "aiLevelSheet.tga", true );

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
        parentServeCloneDownloadButton = new Button( 
            font16, 
            translate( "button_parent_clone" ),
            128,
            106 );
        }
    else {
        // just a straight hosting button
    
        parentServeCloneDownloadButton = NULL;
        
        parentButton = new Button( font16, translate( "button_parent" ),
                                   128,
                                   106 );
        }
    
    childButton = new Button( font16, translate( "button_child" ),
                                                 128,
                                                 146 );

    helpButton = new Button( "helpToggle.tga",
                             242,
                             178,
                             18, 18 );


    nextSongActButton = new Button( font16, translate( "button_nextSongAct" ),
                                    38, 87 );

    songRerollButton = new Button( font16, translate( "button_songReroll" ),
                                    128, 87 );

    switchSongButton = new Button( font16, translate( "button_switchSongs" ),
                                   218, 87 );





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

    printOut( "  ++++++  Init watch display\n" );
    initWatch();
    
    


    resetStats();
    

    
            

    initMusic();

    // start off limiting music
    limitTotalMusicTracks( true );
    
    if( ! isAutoconnecting() ) {
        currentGameState = pickGameTypeState;
        }
    else {
        titleFade = 0;
        currentGameState = connectState;
        
        // we skip the pickGameTypeState, so set this here
        networkOpponent = true;
        }


    // each game starts with inspector in a different spot
    setMusicBasedOnGameState();    

    
    //currentGameState = connectState;
    //currentGameState = sellDiamondsState;
    currentGameState->enterStateCall();

    stillLoading = false;

    // discard any touch that is lingering from activity during load screen
    int tx, ty;
    getTouch( &tx, &ty );
    }



void gameFree() {
    delete font8;
    delete font16;
    delete font16Hand;
    delete doneButton;
    delete nextButton;
    delete backButton;
    delete playAgainButton;
    delete helpButton;
    
    delete nextSongActButton;
    delete songRerollButton;
    delete switchSongButton;
    
    delete aiButton;
    delete wifiButton;
    
    if( parentServeCloneDownloadButton != NULL ) {
        delete parentServeCloneDownloadButton;
        parentServeCloneDownloadButton = NULL;
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
    
    freeWatch();
    

    freeOpponent();

    freeMusic();

    freeHelp();
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

    setMusicBasedOnGameState();    

    resetAI();
    }


    
// track whether current state has finished
static char stateDone = false;



static char shouldGoToConnectionBrokenState() {
    
    if( currentGameState != pickGameTypeState &&
        currentGameState != setAILevelState &&
        currentGameState != connectState &&
        currentGameState != connectionBrokenState ) {
        
        if( checkOpponentConnectionStatus() != 1 ) {
            return true;
            }
        }
    
    return false;
    }





// only register a click when stylus/mouse lifted
char touchDownOnLastCheck = false;
int lastTouchX, lastTouchY;


static void goToNextGameState() {
    // state transition

    // special case if connection ever broken:
    // go right to connectionBrokenState from ANY state
    if( shouldGoToConnectionBrokenState() ) {
        currentGameState = connectionBrokenState;

        // back to limiting music
        limitTotalMusicTracks( true );
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
        // enable full music now
        limitTotalMusicTracks( false );
        }
    else if( currentGameState == connectState ) {
        currentGameState = accumulateDiamondsState;
        // enable full music now
        limitTotalMusicTracks( false );
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
    else if( currentGameState == gameEndState ) {

        if( gameEndState->getParameter() ) {
            // play again!
            currentGameState = accumulateDiamondsState;
            
            // enable full music now
            limitTotalMusicTracks( false );
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

                
    setMusicBasedOnGameState();

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
              
    setMusicBasedOnGameState();
  
    currentGameState->enterStateCall();
    }

    

void gameLoopTick() {
    if( stillLoading ) {
        
        // no tick
        return;
        }
    

    if( ! soundPansSet ) {
        
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
        soundPansSet = true;
        }

    if( globalVolumeRise && globalSoundVolume < 127 ) {
        
        globalSoundVolume++;
        
        for( int c=0; c<8; c++ ) {
            setSoundChannelVolume( c, globalSoundVolume );
            }
        }
    else if( !globalVolumeRise && globalSoundVolume > 0 ) {
        
        globalSoundVolume--;
        
        for( int c=0; c<8; c++ ) {
            setSoundChannelVolume( c, globalSoundVolume );
            }
        }
    


    stepSprites();
    stepOpponent();
    
    
    if( shouldGoToConnectionBrokenState() ) {
        // connection broken, don't even wait for DONE

        isWaitingOnOpponent = false;

        // what if help is in the way?
        if( isHelpShowing() ) {
            forceHideHelp();
            }
        // go immediately to next state (the connection BROKEN state)
        goToNextGameState();
        }
    else if( currentGameState->isStateDone() ) {
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

        // only step if not done and help not showing
        if( ! isHelpShowing() ) {
            currentGameState->stepState();
            }
        }
    
        
    if( isHelpShowing() ) {
        stepHelp();
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
        char touchEaten = false;
        
        // this achieves the equivalent of a "mouse released" event
        

        if( isHelpShowing() ) {
            clickHelp( tx, ty );
            touchEaten = true;
            }
        

        // don't pass clicks to done states
        if( !touchEaten && !currentGameState->isStateDone() ) {
                        
            if( currentGameState->canStateBeBackedOut() ) {
                
                if( backButton->getPressed( tx, ty ) ) {
                    touchEaten = true;
                    
                    goToPreviousGameState();
                    // reset the frame counter after each BACK
                    frameCounter = 0;
                    }
                }

            if( ! touchEaten && currentGameState->canShowHelp() && 
                ! isHelpShowing() ) {
                
                // help button
                if( helpButton->getPressed( tx, ty ) ) {
                    touchEaten = true;
                    showHelp( currentGameState->getHelpTransKey() );
                    }
                }
            
                
            
            if( ! touchEaten ) {
                // pass through to state
                currentGameState->clickState( tx, ty );
                }
            }
        else if( !touchEaten && currentGameState != gameEndState ) {
        
            // next button visible

            if( nextButton->getPressed( tx, ty ) ) {
                touchEaten = true;
                
                goToNextGameState();
                // reset the frame counter after each NEXT
                frameCounter = 0;
                }
            
            }
        
        
        }

    

    if( isWaitingOnOpponent ) {
        stepWatch();
        }
    else {
        resetWatch();
        }
    


    if( getLidClosed() ) {
        stepsSinceLidClosedMusicChange ++;

        if( stepsSinceLidClosedMusicChange >= 
            stepsToWaitBeforeLidClosedMusicChange ) {
            
            // force the music to change so it doesn't get boring while
            // the lid is closed

            // set to a random state
            char newState[12];
                
            newState[11] = '\0';
                
            for( int i=0; i<11; i++ ) {
                newState[i] = (char)( 'a' + getRandom( 26 ) );
                }
            
            setMusicState( newState );
            

            stepsSinceLidClosedMusicChange = 0;
            
            // change music every 5 to 25 seconds
            stepsToWaitBeforeLidClosedMusicChange = 
                (int)( 30 * ( 5 + getRandom( 20 ) ) );
            }
        }
    else {
        stepsSinceLidClosedMusicChange = 0;
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
    


    
    if( stillLoading ) {
        int monthsLeftForLoading = 4;

        if( font16 != NULL ) {
            
            //printOut( "Drawing LOADING message on top screen\n" );
            
            drawGreenBarPaper( 148, 191 );
            
            startNewSpriteLayer();

            font16->drawString( translate( "status_loading" ), 
                                greenBarLeftMargin, 
                                166, 
                                getGreenBarInkColor( 166 - 148, 
                                                     monthsLeftForLoading, 
                                                     false ), 
                                alignLeft );
            }
        else {
            printOut( "Large font not loaded, so not drawing "
                      "LOADING message\n" );
            }
        
 
        if( font8 != NULL ) {
            //printOut( "Progress: %s\n", loadingProgress );

            font8->drawString( loadingProgress, 
                               greenBarLeftMargin, 
                               183,
                               getGreenBarInkColor( 183 - 148, 
                                                    monthsLeftForLoading, 
                                                    true ), 
                               alignLeft );
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
        
        const char *headerString;
        
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
                            greenBarLeftMargin, 
                            151, 
                            getGreenBarInkColor( 151 - 148, 
                                                 getMonthsLeft(), 
                                                 false ), 
                            alignLeft );
        
        font16->drawString( statusMessage, 
                            greenBarLeftMargin, 
                            166, 
                            getGreenBarInkColor( 166 - 148, 
                                                 getMonthsLeft(), 
                                                 false ), 
                            alignLeft );

        if( statusSubMessage != NULL ) {
            font8->drawString( statusSubMessage, 
                               greenBarLeftMargin, 
                               183,
                               getGreenBarInkColor( 183 - 148, 
                                                    getMonthsLeft(), 
                                                    true ), 
                               alignLeft );
            }

        if( isWaitingOnOpponent && ! networkOpponent ) {
            
            #define AI_PROG_LENGTH   17

            int progress = getAIProgress( AI_PROG_LENGTH );
            
            if( progress > 0 ) {
                
                // don't START showing a progress bar when we've only
                // got a few steps left, because this causes a visual
                // flicker
                
                if( isAIProgressShowing ||
                    getAIStepsLeft() > 4 ) {
                    
                    isAIProgressShowing = true;
                    

                    // After AI finishes, we wait until state is done
                    // before isWaitingOnOpponent resets
                    // don't show an empty bar after AI finishes!


                    // show a progress bar for the AI
                
                    char progressString[ AI_PROG_LENGTH + 1 ];
                
                    progressString[ AI_PROG_LENGTH ] = '\0';
                

                    // looks the same as the loading bar
                    memset( progressString, ' ', AI_PROG_LENGTH + 1 );
                
                    memset( progressString, ')', (unsigned int)progress );
                
                
                    font8->drawString( progressString, 
                                       // takes up right half of page
                                       // leaving room for sub-status message
                                       169, 
                                       183,
                                       getGreenBarInkColor( 183 - 148, 
                                                            getMonthsLeft(), 
                                                            true ),
                                       alignLeft );
                    }
                }
            }
        else {
            // keep reset until next display of progress bar
            isAIProgressShowing = false;
            }
        
        
        }
    
        


    
    // drawing a sat phone under the signal strength icon was not approved 
    // by Nintendo
    int sigStrength = getSignalStrength();
        

    if( sigStrength >= 0 && sigStrength <= 3 ) {
                    
        startNewSpriteLayer();
        
        drawSprite( signalSpriteID[ sigStrength ], 
                    232, 121, white );
        }
    
    
    if( isWaitingOnOpponent && networkOpponent
        ||
        // avoid flicker when showing clock for AI, just like we do for
        // AI progress bar
        isAIProgressShowing ) {
        
        drawWatch();
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
            stateString = stringDuplicate( "" );
            }
        
        char *actString = autoSprintf( "Act: %d    Seed string: '%s' ", 
                                       getSongAct(), stateString );
        delete [] stateString;
        

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



        
        char *gridTimeString = getGridStepTimeString();
        
        char *timeLeftString = autoSprintf( "Grid step time: %s", 
                                            gridTimeString );
        delete [] gridTimeString;
        
        font8->drawString( timeLeftString, 128, 49 + 72, white, alignLeft );

        delete [] timeLeftString;


        int secLeft = getSongTimeLeft() / SOUND_SAMPLE_RATE;
        int minLeft = secLeft / 60;
        secLeft = secLeft % 60;
        
        timeLeftString = autoSprintf( "Song time left: %d:%02d", 
                                            minLeft, secLeft );
        
        font8->drawString( timeLeftString, 128, 49 + 84, white, alignLeft );

        delete [] timeLeftString;
        }
    
    }


void drawBottomScreen() {
    
    if( stillLoading ) {
        // draw credits?

        if( font16 != NULL && font8 != NULL ) {

            drawGreenBarPaper( bottomGreenbarSheetTop, 191 );
            
            startNewSpriteLayer();


            int monthsLeftForCredits = 4;

            // space out credits evenly down page, relative to page top
            // so that they move with page as it scrolls up

            font16->drawString( translate( "credit_1a" ), 
                                greenBarLeftMargin, 
                                19 + bottomGreenbarSheetTop, 
                                getGreenBarInkColor( 19, 
                                                     monthsLeftForCredits, 
                                                     false ), 
                                alignLeft );
                
            font8->drawString( translate( "credit_1b" ), 
                               greenBarLeftMargin, 
                               37 + bottomGreenbarSheetTop, 
                               getGreenBarInkColor( 36, 
                                                    monthsLeftForCredits, 
                                                    true ),
                               alignLeft );




            font8->drawString( translate( "credit_2a" ), 
                               greenBarLeftMargin, 
                               69 + bottomGreenbarSheetTop, 
                               getGreenBarInkColor( 68, 
                                                    monthsLeftForCredits, 
                                                    true ),
                               alignLeft );

            font16->drawString( translate( "credit_2b" ), 
                                greenBarLeftMargin, 
                                83 + bottomGreenbarSheetTop,
                                getGreenBarInkColor( 83, 
                                                     monthsLeftForCredits, 
                                                     false ), 
                                alignLeft );
                
            

    
            font8->drawString( translate( "credit_3a" ), 
                               greenBarLeftMargin, 
                               117 + bottomGreenbarSheetTop, 
                               getGreenBarInkColor( 116, 
                                                    monthsLeftForCredits, 
                                                    true ),
                               alignLeft );

            font16->drawString( translate( "credit_3b" ), 
                                greenBarLeftMargin, 
                                131 + bottomGreenbarSheetTop,
                                getGreenBarInkColor( 131, 
                                                     monthsLeftForCredits, 
                                                     false ), 
                                alignLeft );
                
            }

        return;
        }
    
    if( !isHelpShowing() || getHelpScrollProgress() < helpFadeStepsMax ) {
        
        currentGameState->drawState();


        if( currentGameState->isStateDone() 
            && currentGameState->needsNextButton()
            && currentGameState != gameEndState ) {
            
            nextButton->draw();
            }
        else if( ! currentGameState->isStateDone() ) {
            if( currentGameState->canStateBeBackedOut() ) {        
                backButton->draw();
                }
            
            if( currentGameState->canShowHelp() ) {
                helpButton->draw();
                }
            }
        
        startNewSpriteLayer();

        if( isHelpShowing() ) {
            // cover with black rectangle to fade out
            rgbaColor fadingBlack = 
                { 0, 0, 0, 
                  (unsigned char)(
                      ( getHelpScrollProgress() * 255 ) / helpFadeStepsMax ) };
            
            drawRect( 0, 0, 256, 192, fadingBlack );
        
            startNewSpriteLayer();
            }
        
        }
    
    if( isHelpShowing() ) {
        drawHelp();
        }
    
    }



void updateLoadingProgress() {

    runGameLoopOnce();
    runGameLoopOnce();
    
    // add another tick to progress
    for( int i=0; i<PROGRESS_LENGTH; i++ ) {
        if( loadingProgress[i] == ' ' ) {
            loadingProgress[i] = ')';
            printOut( "Setting new tick at %d\n", i );
            currentLoadingProgress = i;
            break;
            }
        }

    if( currentLoadingProgress == PROGRESS_LENGTH / 3 ) {
        scrollCreditSheetUp();
        }
    else if( currentLoadingProgress == 2 * PROGRESS_LENGTH / 3 ) {
        scrollCreditSheetUp();
        }
    }



static int creditScrollCount = 0;

static int creditScrollCurrentSpeed = 0;


static void scrollCreditSheetUp() {
    int lastSheetTop = bottomGreenbarSheetTop;
    
    int scrollAmount = 64;
    

    if( creditScrollCount != 0 ) {
        // only use 64 for first scroll
        scrollAmount = 48;
        }
    // note that this does NOT fill the screen with a full page.

    while( bottomGreenbarSheetTop > lastSheetTop - scrollAmount ) {
        
        if( bottomGreenbarSheetTop < lastSheetTop - scrollAmount + 10 ) {
            // start slowing down near end of scroll
            if( creditScrollCurrentSpeed > 2 ) {
                creditScrollCurrentSpeed -= 2;
                }
            }
        else if( creditScrollCurrentSpeed < 4 ) {
            // speed up
            creditScrollCurrentSpeed += 2;
            }

        bottomGreenbarSheetTop -= creditScrollCurrentSpeed;
        
        if( bottomGreenbarSheetTop == lastSheetTop - scrollAmount ) {
            creditScrollCurrentSpeed = 0;
            }

        runGameLoopOnce();
        runGameLoopOnce();
        }

    creditScrollCount ++;
    }


    
