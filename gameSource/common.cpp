
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
unsigned int intSqrt( unsigned int inX ) {
    unsigned int op, res, one;

    op = inX;
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


// integer arctan2 found here:
// http://cboard.cprogramming.com/c-programming/110593-trig-functions-without-math-h.html#post820808
int intArctan2( int inX, int inY ) {
   int angle;
   int absY = intAbs( inY );

   if( inX >= 0 ) {
       angle = ( 45 - ( 45 * ( inX - absY ) / ( inX + absY ) ) );
       }
   else {
       angle = ( 135 - ( 45 * ( inX + absY ) / ( absY - inX ) ) );
       }
   

   if( inY < 0 ) {
       // negate if in quad III or IV
       return 360 - angle;     
       }
   else {
       return angle;
       }
    }



int intDistance( intPair inA, intPair inB ) {
    int dx = inA.x - inB.x;
    int dy = inA.y - inB.y;
    
    return (int)intSqrt( (unsigned int)( dx * dx + dy * dy ) );
    }



int intAbs( int inX ) {
    if( inX < 0 ) {
        return -inX;
        }
    return inX;
    }


char equals( intPair inA, intPair inB ) {
    return inA.x == inB.x && inA.y == inB.y;
    }



intPair add( intPair inA, intPair inB ) {
    intPair returnVal;
    returnVal.x = inA.x + inB.x;
    returnVal.y = inA.y + inB.y;
    
    return returnVal;
    }



intPair subtract( intPair inA, intPair inB ) {
    intPair returnVal;
    returnVal.x = inA.x - inB.x;
    returnVal.y = inA.y - inB.y;
    
    return returnVal;
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

    if( returnID == -1 ) {
        printOut( "ERROR:  Failed to load sprite from file %s\n", inFileName );
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



rgbaColor *extractRegion( rgbaColor *inImage, int inW, int inH,
                          int inOffsetX, int inOffsetY, 
                          int inExtractW, int inExtractH ) {
    rgbaColor *extracted = new rgbaColor[ inExtractW * inExtractH ];
    
    for( int y=0; y<inExtractH; y++ ) {
        int imageY = y + inOffsetY;
        for( int x=0; x<inExtractW; x++ ) {
            
            extracted[ y * inExtractW + x ] =
                inImage[ imageY * inW + x + inOffsetX ];
            }
        }

    return extracted;
    }



