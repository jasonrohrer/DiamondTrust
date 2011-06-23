#include "RGBAFilter.h"


class BluePenRGBAFilter : public RGBAFilter {
    public:
        
        virtual void filter( rgbaColor *inPixels, 
                             int inWidth, int inHeight );
    };

