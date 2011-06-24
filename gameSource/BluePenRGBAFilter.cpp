#include "BluePenRGBAFilter.h"
#include "platform.h"
#include "colors.h"
#include "common.h"

//static rgbaColor darkBlueInk = {43, 51, 130, 255}
//static rgbaColor darkBlueInk = {0, 0, 130, 255};
//static rgbaColor darkBlueInk = {0, 0, 192, 255};
static rgbaColor darkBlueInk = {43, 51, 192, 255};



void BluePenRGBAFilter::filter( rgbaColor *inPixels, 
                                int inWidth, int inHeight ) {
    
    // first, invert the whole thing (for RGB, not A)
    // make it black ink on white
    
    int numPixels = inWidth * inHeight;
    
    for( int p=0; p<numPixels; p++ ) {
        inPixels[p].r = (unsigned char)( 255 - inPixels[p].r );
        inPixels[p].g = (unsigned char)( 255 - inPixels[p].g );
        inPixels[p].b = (unsigned char)( 255 - inPixels[p].b );
        }
    
    // next, blend between dark blue and white based on pixel's gray color
    // preserve alpha
    for( int p=0; p<numPixels; p++ ) {
        unsigned char originalA = inPixels[p].a;
        
        inPixels[p] = blendColors( white, darkBlueInk, inPixels[p].r );
        inPixels[p].a = originalA;
        }
    }

