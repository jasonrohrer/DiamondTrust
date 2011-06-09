#include "GameState.h"
#include "Button.h"
#include "common.h"
#include "gameStats.h"
#include "units.h"
#include "colors.h"
#include "music.h"

#include "minorGems/util/stringUtils.h"


static char stateDone = false;

extern int satelliteTopSpriteID;
extern int satelliteBottomSpriteID;
extern int satelliteBottomHalfOffset;

extern int titleSpriteID;
extern unsigned char titleFade;


extern Button *nextSongActButton;

extern char allowManualSongActSwitching;



extern Button *aiButton;
extern Button *wifiButton;
extern char *statusMessage;
extern char *statusSubMessage;

extern char networkOpponent;



class PickGameTypeState : public GameState {
    public:
        PickGameTypeState();
        
        virtual void clickState( int inX, int inY );
        

        virtual void stepState();
        
        

        // draws into bottom screen
        virtual void drawState();


        virtual void enterState();        
        virtual char isStateDone() {
            return stateDone;
            }
        
        virtual char needsNextButton() {
            // auto-advance
            return false;
            }
        

        
        virtual ~PickGameTypeState();
        
    protected:
        char *mMessage;
    };


PickGameTypeState::PickGameTypeState() 
        : mMessage( NULL ) {
    mStateName = "PickGameTypeState";
    }

PickGameTypeState::~PickGameTypeState() {
    
    if( mMessage != NULL ) {
        delete [] mMessage;
        }
    }

static char selectionMade = false;



void PickGameTypeState::clickState( int inX, int inY ) {

 
    if( ! selectionMade ) {
        if( aiButton->getPressed( inX, inY ) ) {
            selectionMade = true;
            
            networkOpponent = false;
            }
        else if( wifiButton->getPressed( inX, inY ) ) {
            selectionMade = true;
            
            networkOpponent = true;
            initOpponent( false );
            }
        else if( allowManualSongActSwitching &&
                 nextSongActButton->getPressed( inX, inY ) ) {
            nextSongAct();
            }
        }
    }



void PickGameTypeState::stepState() {

    if( selectionMade ) {
        if( titleFade > 0 ) {
        
            if( titleFade >= 8 ) {
                titleFade -= 8;
                }
            else {
                titleFade = 0;
                }
            }
        }
    else {
        if( titleFade < 255 ) {
            if( titleFade <= 255 - 8 ) {
                titleFade += 8;
                }
            else {
                titleFade = 255;
                }
            }
        }

    if( selectionMade && titleFade == 0 ) {
        stateDone = true;
        }
    }



void PickGameTypeState::drawState() {
    
    drawSprite( satelliteTopSpriteID, 
                0,0, white );
    drawSprite( satelliteBottomSpriteID, 
                0,satelliteBottomHalfOffset, white );
    
    startNewSpriteLayer();
    
    if( titleFade > 0 ) {
        rgbaColor titleColor = white;
        titleColor.a = titleFade;
        
        drawSprite( titleSpriteID, 
                    0,6, titleColor );
        /*
        drawSprite( titleSpriteID[1], 
                    0,64, titleColor );

        drawSprite( titleSpriteID[2], 
                    0,80, titleColor );
        */
        }

    
    
    if( !selectionMade && titleFade == 255 ) {
        
        aiButton->draw();
        wifiButton->draw();
        
        if( allowManualSongActSwitching ) {
            nextSongActButton->draw();
            }
        }
    
    }





void PickGameTypeState::enterState() {
    stateDone = false;

    statusMessage = translate( "phaseStatus_gameType" );
    statusSubMessage = translate( "phaseSubStatus_gameType" ); 
    
    selectionMade = false;

    }






// singleton
static PickGameTypeState state;


GameState *pickGameTypeState = &state;

