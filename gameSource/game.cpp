

#include "platform.h"
#include "tga.h"


void gameInit() {
    int fileDataSize;
    unsigned char *spriteFileData = readFile( "testTexture.tga", 
                                              &fileDataSize );
    if( spriteFileData != NULL ) {
        
        int width, height;
        
        rgbaColor *spriteRGBA = extractTGAData( spriteFileData, fileDataSize,
                                                &width, &height );
        
        
        freeMem( spriteFileData );
        }
    
    }


void gameLoopTick() {}


void drawTopScreen() {}

void drawBottomScreen() {}
