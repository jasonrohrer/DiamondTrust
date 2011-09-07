
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


// O(log inY) algorithm found here:
// http://stackoverflow.com/questions/101439/
//  the-most-efficient-way-to-implement-an-integer-based-power-
//  function-powint-int
// this might actually have more overhead for smaller exponents
/*
int intPower( int inX, unsigned int inY ) {
    int result = 1;
    while( inY ) {
        if( inY & 1 ) {
            result *= inX;
            }
        inY >>= 1;
        inX *= inX;
        }
    return result;
    }
*/

// my original O( inY ) implementation
int intPower( int inX, unsigned int inY ) {
    int res = 1;
    
    for( unsigned int y=0; y<inY; y++ ) {
        res *= inX;
        }

    return res;
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









int loadSprite( const char *inFileName, char inCornerTransparent ) {
    int w, h;

    return loadSprite( inFileName, &w, &h, inCornerTransparent );
    }






int loadSprite( const char *inFileName, int *outW, int *outH, 
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



void loadTiledSprites( const char *inFileName, int inNumParts,
                       int *outSpriteIDs, 
                       intPair *inOffsets, intPair *inSizes,
                       char inCornerTransparent,
                       rgbaColor *inCustomTransparentColor ) {

    int w, h;
    
    rgbaColor *spriteRGBA = readTGAFile( inFileName, &w, &h );

    if( spriteRGBA == NULL ) {
        
        printOut( "Reading sprite from %s failed.\n", inFileName );
        return;
        }
    
    if( inCustomTransparentColor != NULL ) {
        applyCutomTransparency( spriteRGBA, w * h, inCustomTransparentColor );
        }
    else if( inCornerTransparent ) {    
        applyCornerTransparency( spriteRGBA, w * h );
        }
    

    for( int p=0; p<inNumParts; p++ ) {
        intPair offset = inOffsets[p];
        intPair size = inSizes[p];
        
        rgbaColor *partRGBA = extractRegion( spriteRGBA, 
                                             w, h,
                                             offset.x, offset.y,
                                             size.x, size.y );
        outSpriteIDs[p] = addSprite( partRGBA, size.x, size.y );
    
        delete [] partRGBA;
        }

    delete [] spriteRGBA;

    }






void applyCornerTransparency( rgbaColor *inImage, int inNumPixels ) {
    rgbaColor transColor = inImage[0];
    
    applyCutomTransparency( inImage, inNumPixels, &transColor );
    }



void applyCutomTransparency( rgbaColor *inImage, int inNumPixels,
                             rgbaColor *inCustomTransparentColor ) {
    unsigned char tr, tg, tb;
    tr = inCustomTransparentColor->r;
    tg = inCustomTransparentColor->g;
    tb = inCustomTransparentColor->b;
    
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
    
    if( inOffsetY + inExtractH > inH
        || 
        inOffsetX + inExtractW > inW ) {
        printOut( "Error:  extractRegion out of image boundaries\n" );
        return NULL;
        }
    

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



rgbaColor blendColors( rgbaColor inA, rgbaColor inB, 
                       unsigned char inAWeight ) {
    
    int aWeight = inAWeight;
    int bWeight = 255 - inAWeight;

    rgbaColor returnColor;
    
    returnColor.r = 
        (unsigned char)( ( inA.r * aWeight + inB.r * bWeight ) / 255 );
    returnColor.g = 
        (unsigned char)( ( inA.g * aWeight + inB.g * bWeight ) / 255 );
    returnColor.b = 
        (unsigned char)( ( inA.b * aWeight + inB.b * bWeight ) / 255 );
    returnColor.a = 
        (unsigned char)( ( inA.a * aWeight + inB.a * bWeight ) / 255 );
    
    return returnColor;
    }



char *readFileAsString( const char *inFileName ) {
    int dataSize;
    unsigned char *data = readFile( inFileName, &dataSize );
    
    if( data != NULL ) {
        char *textData = new char[ dataSize + 1 ];
        memcpy( textData, data, (unsigned int)dataSize );
        textData[ dataSize ] = '\0';
        
        delete [] data;
    
        return textData;
        }

    return NULL;
    }

    


