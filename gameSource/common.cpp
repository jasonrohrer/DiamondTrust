
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





// integer square root found here:
// http://www.codecodex.com/wiki/Calculate_an_integer_square_root
unsigned long intSqrt( unsigned long x ) {
    unsigned long op, res, one;

    op = x;
    res = 0;
    
    // "one" starts at the highest power of four <= than the argument. 
    one = 1 << 30;  // second-to-top bit set 
    while( one > op ) one >>= 2;

    while( one != 0 ) {
        if( op >= res + one ) {
            op = op - ( res + one );
            res = res +  2 * one;
            }
        res >>= 1;
        one >>= 2;
        }
    return( res );
    }


int intDistance( intPair inA, intPair inB ) {
    int dx = inA.x - inB.x;
    int dy = inA.y - inB.y;
    
    return intSqrt( dx * dx + dy * dy );
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


