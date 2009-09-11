#include "sprite.h"



int blinkingSpriteAlpha = 255;
int blinkingSpriteMin = 64;

int blinkingSpriteDelta = -18;


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
    }

    

void drawBlinkingSprite( int inHandle, 
                         int inX, int inY, rgbaColor inColor ) {
    
    inColor.a = (unsigned char)blinkingSpriteAlpha;
    
    drawSprite( inHandle, inX, inY, inColor );
    }

