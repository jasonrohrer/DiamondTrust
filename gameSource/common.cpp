
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
            // use corner color as transparency
            spriteRGBA[0].a = 0;
            unsigned char tr, tg, tb;
            tr = spriteRGBA[0].r;
            tg = spriteRGBA[0].g;
            tb = spriteRGBA[0].b;

            int numPixels = width * height; 
            for( int i=0; i<numPixels; i++ ) {
                if( spriteRGBA[i].r == tr 
                    &&
                    spriteRGBA[i].g == tg 
                    &&
                    spriteRGBA[i].b == tb ) {
                    
                    spriteRGBA[i].a = 0;
                    }
                }
            }
            
            
            
                        
                        
        returnID = addSprite( spriteRGBA, width, height );

        *outW = width;
        *outH = height;
            
        delete [] spriteRGBA;
        }
    return returnID;
    }

