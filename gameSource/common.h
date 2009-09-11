#include "platform.h"


typedef struct intPair {
        int x;
        int y;
    } intPair;


// finds power of 2 that value fits inside
// returns inValue if it is already power of 2
int roundUpToPowerOfTwo( int inValue );



inline char equals( rgbaColor inA, rgbaColor inB ) {
    return 
        inA.r == inB.r && 
        inA.g == inB.g && 
        inA.b == inB.b && 
        inA.a == inB.a;
    }



inline void setColor( rgbaColor *inColor,
                      unsigned char inR, unsigned char inG, 
                      unsigned char inB, unsigned char inA ) {
    inColor->r = inR;
    inColor->g = inG;
    inColor->b = inB;
    inColor->a = inA;
    }

                      
