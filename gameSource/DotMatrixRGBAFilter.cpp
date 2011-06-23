#include "DotMatrixRGBAFilter.h"
#include "platform.h"


void DotMatrixRGBAFilter::filter( rgbaColor *inPixels, 
                                  int inWidth, int inHeight ) {
    
    // first, make the whole thing black (for RGB, not A)

    int numPixels = inWidth * inHeight;
    
    for( int p=0; p<numPixels; p++ ) {
        inPixels[p].r = 0;
        inPixels[p].g = 0;
        inPixels[p].b = 0;        
        }
    
    // for each scanline, vary darkness slighly
    // avoid using too many colors (increases texture size)
    
    for( int y=0; y<inHeight; y++ ) {

        // pick a value from {63, 159, 255}
        // never pure black
        unsigned char inkDarkness = 
            (unsigned char)( 96  * getRandom( 3 ) + 63 );


        
        int p = y * inWidth;
            
        for( int x=0; x<inWidth; x++ ) {
            
            inPixels[p].r = inkDarkness;
            inPixels[p].g = inkDarkness;
            inPixels[p].b = inkDarkness;
            
            p++;
            }
        }
    

    }

