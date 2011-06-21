#ifndef RGBAFILTER_INCLUDED
#define RGBAFILTER_INCLUDED


#include "minorGems/graphics/rgbaColor.h"


// interface for RGBA filters


class RGBAFilter {
        
    public:
        
        virtual ~RGBAFilter() {
            };
        

        virtual void filter( rgbaColor *inPixels, 
                             int inWidth, int inHeight ) = 0;
        
    };


#endif
