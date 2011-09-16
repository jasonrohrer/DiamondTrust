#include "pause.h"
#include "Button.h"
#include "colors.h"

#include "help.h"


extern int satelliteTopSpriteID;
extern int satelliteBottomSpriteID;
extern int satelliteBottomHalfOffset;
extern unsigned char satelliteFade;


extern Button *continueButton;
extern Button *quitButton;




static char pauseShowing = false;

static char quitChosen = false;

static char pauseClosing = false;

char isPauseShowing() {
    return pauseShowing;
    }



char isQuitChosen() {
    char returnValue = quitChosen;
    quitChosen = false;
    return returnValue;
    }



void showPause() {
    pauseShowing = true;
    pauseClosing = false;
    quitChosen = false;
    }


void forceHidePause() {
    //pauseClosing = true;
    // instantly hide it
    pauseShowing = false;
    }



void clickPause( int inX, int inY ) {
    if( satelliteFade == 255 ) {
    
        if( continueButton->getPressed( inX, inY ) ) {
            pauseClosing = true;
            }
        else if( quitButton->getPressed( inX, inY ) ) {
            quitChosen = true;
            }
        }
    }

        

void stepPause() {

    if( isHelpShowing() ) {
        // wait for help to be totally put away before we start
        // our fade-in
        return;
        }
    

    if( ! pauseClosing ) {    
        if( satelliteFade < 255 ) {
            
            if( satelliteFade <= 251 ) {
                satelliteFade += 4;
                }
            else {
                satelliteFade = 255;
                }
            }
        }
    else {
        if( satelliteFade > 0 ) {
            
            if( satelliteFade > 4 ) {
                satelliteFade -= 4;
                }
            else {
                satelliteFade = 0;
                pauseShowing = false;
                }
            }
        }
    

    }


        

// draws into bottom screen
void drawPause() {
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
        
        continueButton->draw();
        quitButton->draw();
        }
    }
