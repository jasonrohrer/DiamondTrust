#include "GameState.h"
#include "Button.h"
#include "common.h"
#include "gameStats.h"
#include "units.h"
#include "colors.h"
#include "music.h"
#include "opponent.h"

#include "minorGems/util/stringUtils.h"


static char stateDone = false;

extern int satelliteTopSpriteID;
extern int satelliteBottomSpriteID;
extern int satelliteBottomHalfOffset;

extern int titleSpriteID;
extern unsigned char titleFade;


extern Button *nextSongActButton;
extern Button *songRerollButton;
extern Button *switchSongButton;
extern Button *songPlusButton;
extern Button *songMinusButton;
extern Button *closeLidButton;
extern Button *openLidButton;
extern Button *muteButton;
extern Button *unmuteButton;
extern Button *volPlusButton;
extern Button *volMinusButton;


extern char soundMuted;


extern char allowManualSongActSwitching;
extern char manualLidClosed;

extern Font *font8;


extern Button *aiButton;
extern Button *wifiButton;
extern const char *statusMessage;
extern const char *statusSubMessage;

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
        else if( allowManualSongActSwitching ) {
            if( nextSongActButton->getPressed( inX, inY ) ) {
                if( getSongAct() < 3 ) {
                    nextSongAct();
                    }
                else {
                    backToFirstSongAct();
                    }
                }
            else if( songRerollButton->getPressed( inX, inY ) ) {   
                
                char newState[12];
                
                newState[11] = '\0';
                
                for( int i=0; i<11; i++ ) {
                    newState[i] = (char)( 'a' + getRandom( 26 ) );
                    }
                
                setMusicState( newState );
                }
            else if( switchSongButton->getPressed( inX, inY ) ) {
                switchSongs();
                }
            else if( songPlusButton->getPressed( inX, inY ) ) {
                switchSongs( 1 );
                }
            else if( songMinusButton->getPressed( inX, inY ) ) {
                switchSongs( -1 );
                }
            else if( manualLidClosed && 
                     openLidButton->getPressed( inX, inY ) ) {
                manualLidClosed = false;
                }
            else if( ! manualLidClosed && 
                     closeLidButton->getPressed( inX, inY ) ) {
                manualLidClosed = true;
                }         
            else if( soundMuted && 
                     unmuteButton->getPressed( inX, inY ) ) {
                soundMuted = false;
                }
            else if( !soundMuted && 
                     muteButton->getPressed( inX, inY ) ) {
                soundMuted = true;
                }
            else if( volPlusButton->getPressed( inX, inY ) ) {
                setCustomSongVolume( 
                    getCustomSongVolume( getCurrentSongNumber() ) + 1 );
                }
            else if( volMinusButton->getPressed( inX, inY ) ) {
                setCustomSongVolume( 
                    getCustomSongVolume( getCurrentSongNumber() ) - 1 );
                }
            
            
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
            songRerollButton->draw();
            switchSongButton->draw();
            songPlusButton->draw();
            songMinusButton->draw();
            
            if( manualLidClosed ) {
                openLidButton->draw();
                }
            else {
                closeLidButton->draw();
                }    

            if( soundMuted ) {
                unmuteButton->draw();
                }
            else {
                muteButton->draw();
                }

            volPlusButton->draw();
            volMinusButton->draw();


            int labelEnd = 
                font8->drawString( "Song vol:", 2, 2, white, alignLeft );



            int currentSong = getCurrentSongNumber();
            
            for( int s=0; s<10; s++ ) {
                char *volString = autoSprintf( "%d", 
                                               getCustomSongVolume( s ) );    
            
                rgbaColor drawColor = white;
                
                if( currentSong == s ) {
                    // pink
                    drawColor.g = 127;
                    drawColor.b = 127;
                    }
    
                
                font8->drawString( volString, labelEnd + 5 + 14 + 20 * s, 
                                   2, drawColor, alignRight );

                delete [] volString;
                }
            

            }
        }
    
    }





void PickGameTypeState::enterState() {
    stateDone = false;

    statusMessage = translate( "phaseStatus_gameType" );
    statusSubMessage = translate( "phaseSubStatus_gameType" ); 
    
    selectionMade = false;

    // turn off AI in case it was running from a previous game
    // don't want AI picking its next move during our menu screens
    resetOpponent();
    }






// singleton
static PickGameTypeState state;


GameState *pickGameTypeState = &state;

