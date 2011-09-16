char isPauseShowing();


// returns whether quit has been pressed since the last call to isQuitChosen
char isQuitChosen();


void showPause();

// force pauseto close, even if continue button not clicked
void forceHidePause();


void clickPause( int inX, int inY );
        

void stepPause();
        

// draws into bottom screen
void drawPause();
