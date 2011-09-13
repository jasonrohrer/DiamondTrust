#include "sprite.h"



int blinkingSpriteAlpha = 255;
int blinkingSpriteMin = 64;

int blinkingSpriteDelta = -16;
//int blinkingSpriteDelta = -36;

// watch for times when we haven't drawn blinking sprites in a while
// a good chance to reset so that next blinking starts in a good place
int spriteStepsWithNoDraw = 0;



void resetBlinkingSprites() {
    blinkingSpriteAlpha = 255;
    blinkingSpriteDelta = -16;
    }


void stepSprites() {
    blinkingSpriteAlpha += blinkingSpriteDelta;
    if( blinkingSpriteAlpha > 255 ) {
        blinkingSpriteAlpha = 255;
        blinkingSpriteDelta *= -1;
        }
    else if( blinkingSpriteAlpha < blinkingSpriteMin ) {
        blinkingSpriteAlpha = blinkingSpriteMin;
        
        blinkingSpriteDelta *= -1;
        }
    
    spriteStepsWithNoDraw ++;
    
    if( spriteStepsWithNoDraw > 2 ) {
        resetBlinkingSprites();
        }
    }

    

void drawBlinkingSprite( int inHandle, 
                         int inX, int inY, rgbaColor inColor ) {
    
    

    inColor.a = (unsigned char)( ( blinkingSpriteAlpha * inColor.a ) / 255 );
    
    drawSprite( inHandle, inX, inY, inColor );
    
    spriteStepsWithNoDraw = 0;
    }

