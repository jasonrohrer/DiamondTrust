#ifndef COMMON_INCLUDED
#define COMMON_INCLUDED


#include "platform.h"
#include "minorGems/util/TranslationManager.h"


typedef struct intPair {
        int x;
        int y;
    } intPair;


// finds power of 2 that value fits inside
// returns inValue if it is already power of 2
int roundUpToPowerOfTwo( int inValue );


// returns -1 on fail
int loadSprite( char *inFileName, int *outW, int *outH,
                char inCornerTransparent = false );

int loadSprite( char *inFileName, char inCornerTransparent = false );


void applyCornerTransparency( rgbaColor *inImage, int inNumPixels );




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


                      
inline void printColor( rgbaColor inC ) {
    printOut( "(%d,%d,%d,%d)", inC.r, inC.g, inC.b, inC.a );
    }


inline char *translate( char *inKey ) {
    return (char*)TranslationManager::translate( inKey );
    }


#endif
