#include "RGBAFilter.h"


class DotMatrixRGBAFilter : public RGBAFilter {
    public:
        
        virtual void filter( rgbaColor *inPixels, 
                             int inWidth, int inHeight );
    };

