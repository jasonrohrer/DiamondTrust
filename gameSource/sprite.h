#include "platform.h"


void stepSprites();

void resetBlinkingSprites();

void drawBlinkingSprite( int inHandle, 
                         int inX, int inY, rgbaColor inColor );


inline void drawSprite( int inHandle, 
                        int inX, int inY, rgbaColor inColor, 
                        char inBlinking ) {
    if( inBlinking ) {
        drawBlinkingSprite( inHandle, inX, inY, inColor );
        }
    else {
        drawSprite( inHandle, inX, inY, inColor );
        }
    }

