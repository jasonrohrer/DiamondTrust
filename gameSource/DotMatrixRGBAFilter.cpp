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
    // avoid using too many colors (increases texture size
    
    for( int y=0; y<inHeight; y++ ) {
        unsigned char inkDarkness = (unsigned char)( 127  * getRandom( 3 ) );
        
        int p = y * inWidth;
            
        for( int x=0; x<inWidth; x++ ) {
            
            inPixels[p].r = inkDarkness;
            inPixels[p].g = inkDarkness;
            inPixels[p].b = inkDarkness;
            
            p++;
            }
        }
    

    }

