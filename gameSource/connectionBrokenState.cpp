#include "GameState.h"
#include "Button.h"
#include "colors.h"
#include "map.h"
#include "units.h"
#include "gameStats.h"
#include "common.h"

//static int activeUnit = -1;
static char stateDone = false;


extern Button *doneButton;


extern const char *statusMessage;
extern const char *statusSubMessage;


extern int satelliteTopSpriteID;
extern int satelliteBottomSpriteID;
extern int satelliteBottomHalfOffset;
extern unsigned char satelliteFade;






class ConnectionBrokenState : public GameState {
    public:
        
        ConnectionBrokenState() {
            mStateName = "ConnectionBrokenState";
            }
        
        virtual void clickState( int inX, int inY );
        

        virtual void stepState();
        
        

        // draws into bottom screen
        virtual void drawState();


        virtual void enterState();        
        virtual char isStateDone() {
            return stateDone;
            }
        
        virtual char needsNextButton() {
            return false;
            }        
        
    };










void ConnectionBrokenState::clickState( int inX, int inY ) {
    
    if( doneButton->getPressed( inX, inY ) ) {
        stateDone = true;
        }
    }




void ConnectionBrokenState::stepState() {

    // fade satellite back in after Done
    if( satelliteFade < 255 ) {
        
        if( satelliteFade <= 251 ) {
            satelliteFade += 4;
            }
        else {
            satelliteFade = 255;
            }
        }
    }



void ConnectionBrokenState::drawState() {
    drawMap();
    startNewSpriteLayer();
    
    drawUnits();

    startNewSpriteLayer();
    
    if( satelliteFade > 0 ) {
        
        // draw fading satellite map on top
        rgbaColor satelliteColor = white;
        satelliteColor.a = satelliteFade;
        
        drawSprite( satelliteTopSpriteID, 
                    0,0, satelliteColor );
        drawSprite( satelliteBottomSpriteID, 
                    0,satelliteBottomHalfOffset, satelliteColor );
        }


    if( satelliteFade == 255 ) {
        startNewSpriteLayer();
        doneButton->draw();
        }
    }





void ConnectionBrokenState::enterState() {
    stateDone = false;
        
    statusMessage = translate( "phaseStatus_connectionFailed" );
        
    statusSubMessage = "";    


    // reset various displays on disconnect
    showSale( false );
    
    showInspectorPanel( false );
    
    setActiveUnit( -1 );
    }









// singleton
static ConnectionBrokenState state;


GameState *connectionBrokenState = &state;
