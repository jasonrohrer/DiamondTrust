#ifndef FONT_INCLUDED
#define FONT_INCLUDED


#include "platform.h"
#include "RGBAFilter.h"

#include <stdlib.h>

enum TextAlignment {
    alignCenter = 0,
    alignRight,
    alignLeft 
    };


class Font {
        
    public:
        
        // file contains TGA with 16x16 ascii table
        //
        // inFilter filters whole font image BEFORE characters are split
        // from it.  Defaults to NULL
        Font( const char *inFileName, int inCharSpacing, int inSpaceWidth,
              char inFixedWidth, RGBAFilter *inFilter=NULL );
        
        ~Font();
        
        
        // draws a string on the current screen
        // returns x coordinate of string end
        int drawString( const char *inString, 
                        int inX, int inY, rgbaColor inColor,
                        TextAlignment inAlign = alignCenter );


        int measureString( const char *inString );
        

        int getCharacterSpacing();


    private:
        
        // returns x coordinate to right of drawn character
        int drawCharacter( char inC, int inX, int inY, rgbaColor inColor );
        
        
        
        int mCharSpacing;
        int mSpaceWidth;
        
        char mFixedWidth;
        
        int mSpriteWidth;
        
        // maps ascii chars to sprite IDs
        int mSpriteMap[ 128 ];
        
        // for kerning (ignored if fixed width flag on)
        int mCharLeftEdgeOffset[ 128 ];
        int mCharWidth[ 128 ];
        
        
    };


#endif
