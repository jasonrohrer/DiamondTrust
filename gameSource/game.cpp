

#include "platform.h"
#include "tga.h"


int spriteID = -1;

void gameInit() {
    int fileDataSize;
    unsigned char *spriteFileData = readFile( "testTexture.tga", 
                                              &fileDataSize );
    if( spriteFileData != NULL ) {
        
        int width, height;
        
        rgbaColor *spriteRGBA = extractTGAData( spriteFileData, fileDataSize,
                                                &width, &height );
        
        if( spriteRGBA != NULL ) {
            spriteID = addSprite( spriteRGBA, width, height );
            
            freeMem( spriteRGBA );
            }

        freeMem( spriteFileData );
        }
    
    }


void gameLoopTick() {}


void drawTopScreen() {
    rgbaColor c;
    c.r = 255;
    c.g = 255;
    c.b = 255;
    c.a = 255;
    
    drawSprite( spriteID, 100, 100, c );
    }


void drawBottomScreen() {
    rgbaColor c;
    c.r = 255;
    c.g = 255;
    c.b = 255;
    c.a = 255;
    
    drawSprite( spriteID, 100, 100, c );
    }
