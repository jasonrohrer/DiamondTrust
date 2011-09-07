#ifndef COMMON_INCLUDED
#define COMMON_INCLUDED


#include "platform.h"
#include "opponent.h"
#include "TranslationManager.h"


typedef struct intPair {
        int x;
        int y;
    } intPair;


unsigned int intSqrt( unsigned int inX );


// x**y
int intPower( int inX, unsigned int inY );



// computes angle in degrees [0,360] for a vector from (0,0) to (x,y)
// rough integer approximation (off by at most 6 degrees for x or y values in
// range [-256,256]
int intArctan2( int inX, int inY );


int intDistance( intPair inA, intPair inB );

int intAbs( int inX );

char equals( intPair inA, intPair inB );

intPair add( intPair inA, intPair inB );

intPair subtract( intPair inA, intPair inB );



// finds power of 2 that value fits inside
// returns inValue if it is already power of 2
int roundUpToPowerOfTwo( int inValue );


// returns -1 on fail
int loadSprite( const char *inFileName, int *outW, int *outH,
                char inCornerTransparent = false );

int loadSprite( const char *inFileName, char inCornerTransparent = false );


// breaks an image file up into part-sprites to save space in texture ram
// parts specified by inOffsets and inSizes
// outSpriteIDs must be allocated by the caller
// all parameters destroyed by caller
//
// if inCornerTransparentColor not NULL, it is used as the transparent
// color instead of the corner's color
void loadTiledSprites( const char *inFileName, int inNumParts,
                       int *outSpriteIDs, 
                       intPair *inOffsets, intPair *inSizes,
                       char inCornerTransparent,
                       rgbaColor *inCustomTransparentColor = NULL );


void applyCornerTransparency( rgbaColor *inImage, int inNumPixels );

void applyCutomTransparency( rgbaColor *inImage, int inNumPixels,
                             rgbaColor *inCustomTransparentColor );


rgbaColor *extractRegion( rgbaColor *inImage, int inW, int inH,
                          int inOffsetX, int inOffsetY, 
                          int inExtractW, int inExtractH );

                          



inline char equals( rgbaColor inA, rgbaColor inB ) {
    return 
        inA.r == inB.r && 
        inA.g == inB.g && 
        inA.b == inB.b && 
        inA.a == inB.a;
    }

// rgba as 32-bit int
inline unsigned int toInt( rgbaColor inC ) {
    return (unsigned int)( inC.r << 24 | inC.g << 16 | inC.b << 8 | inC.a );
    }



inline void setColor( rgbaColor *inColor,
                      unsigned char inR, unsigned char inG, 
                      unsigned char inB, unsigned char inA ) {
    inColor->r = inR;
    inColor->g = inG;
    inColor->b = inB;
    inColor->a = inA;
    }



rgbaColor blendColors( rgbaColor inA, rgbaColor inB, 
                       unsigned char inAWeight );



                      
inline void printColor( rgbaColor inC ) {
    printOut( "(%d,%d,%d,%d)", inC.r, inC.g, inC.b, inC.a );
    }


inline const char *translate( const char *inKey ) {
    return TranslationManager::translate( inKey );
    }



char *readFileAsString( const char *inFileName );



#endif
