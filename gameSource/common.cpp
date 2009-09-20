
#include "common.h"
#include "tga.h"


int roundUpToPowerOfTwo( int inValue ) {
    int bitSum = 0;
    int highBit = -1;
    
    for( int i=0; i<32; i++ ) {
        int bit = ( inValue >> i ) & 0x1;
        
        bitSum += bit;
        
        if( bit == 1 && i > highBit ) {
            highBit = i;
            }
        }
    if( bitSum == 1 ) {
        return inValue;
        }
    else {
        // flip next higher bit, and leave lower ones at 0
        return 1 << ( highBit + 1 );
        }
    
    }



int loadSprite( char *inFileName, char inCornerTransparent ) {
    int w, h;

    return loadSprite( inFileName, &w, &h, inCornerTransparent );
    }






int loadSprite( char *inFileName, int *outW, int *outH, 
                char inCornerTransparent ) {
    int returnID = -1;
    
    int width, height;
    
    rgbaColor *spriteRGBA = readTGAFile( inFileName,
                                         &width, &height );
        
    if( spriteRGBA != NULL ) {
            
        if( inCornerTransparent ) {

            applyCornerTransparency( spriteRGBA, width * height );
            }
            
        returnID = addSprite( spriteRGBA, width, height );

        *outW = width;
        *outH = height;
            
        delete [] spriteRGBA;
        }
    return returnID;
    }



void applyCornerTransparency( rgbaColor *inImage, int inNumPixels ) {
    // use corner color as transparency
    inImage[0].a = 0;
    unsigned char tr, tg, tb;
    tr = inImage[0].r;
    tg = inImage[0].g;
    tb = inImage[0].b;
    
    for( int i=0; i<inNumPixels; i++ ) {
        if( inImage[i].r == tr 
            &&
            inImage[i].g == tg 
            &&
            inImage[i].b == tb ) {
                    
            inImage[i].a = 0;
            }
        }
    
    }


