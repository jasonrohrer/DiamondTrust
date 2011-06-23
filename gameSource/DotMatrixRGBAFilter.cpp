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
    
    // repeat same pattern of varying darkness for each row of characters
    // (every 16 pixels)

    for( int y=0; y<16; y++ ) {

        // pick a value from {63, 159, 255}
        // never pure black
        unsigned char inkDarkness = 
            (unsigned char)( 96  * getRandom( 3 ) + 63 );


        
        for( int r=0; r*16<inHeight; r++ ) {
            

            int p = ( r * 16 + y ) * inWidth;
            
            for( int x=0; x<inWidth; x++ ) {
                
                inPixels[p].r = inkDarkness;
                inPixels[p].g = inkDarkness;
                inPixels[p].b = inkDarkness;
                
                p++;
                }
            }
        
        }
    

    }

