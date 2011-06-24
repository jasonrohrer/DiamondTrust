#include "help.h"
#include "Button.h"
#include "greenBarPaper.h"


extern Button *helpButton;



// starts off bottom of screen
static int helpGreenbarPageTop = 192;


static char helpShouldShow = false;


static const char *transKey = NULL;



char isHelpShowing() {
    if( helpGreenbarPageTop < 192 ) {
        return true;
        }
    
    // not on screen, but maybe it's trying to come up
    return helpShouldShow;
    }



void showHelp( const char *inHelpTransKey ) {
    helpShouldShow = true;
    
    transKey = inHelpTransKey;
    }



void clickHelp( int inX, int inY ) {
    if( helpButton->getPressed( inX, inY ) ) {
        // toggle
        helpShouldShow = ! helpShouldShow;
        }
    }




void stepHelp() {
    int scrollRate = 8;

    if( helpShouldShow && helpGreenbarPageTop > 0 ) {    
        helpGreenbarPageTop -= scrollRate;
        }
    else if( !helpShouldShow && helpGreenbarPageTop < 192 ) {
        helpGreenbarPageTop += scrollRate;
        }
    }

    

void drawHelp() {
    drawGreenBarPaper( helpGreenbarPageTop, 192 );
    
    startNewSpriteLayer();
    
    helpButton->draw();
    }
