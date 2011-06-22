#include "minorGems/graphics/rgbaColor.h"


void initGreenBarPaper();


void freeGreenBarPaper();


// draws it across screen, starting at x=0
void drawGreenBarPaper( int inSheetTopY, int inBottomY );



// gets ink color for dot-matrix worn-out ribbon effect
// color varies by location on paper (green or white bar
rgbaColor getGreenBarInkColor( int inFontY, int inMonthsLeft, 
                               char inSmallFont );
