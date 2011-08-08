
// help system


// call at program termination
// no init necessary
void freeHelp();


char isHelpShowing();

char isHelpTryingToScroll();


void showHelp( const char *inHelpTransKey );

// force help to close, even if help button not clicked
void forceHideHelp();


void clickHelp( int inX, int inY );
        

void stepHelp();


// from 0 to 192
int getHelpScrollProgress();

        

// draws into bottom screen
void drawHelp();

