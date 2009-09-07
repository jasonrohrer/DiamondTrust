#include "platform.h"


enum TextAlignment {
    alignCenter = 0,
    alignRight,
    alignLeft 
    };


class Font {
        
    public:
        
        // file contains TGA with 16x16 ascii table
        Font( char *inFileName, int inCharSpacing, int inSpaceWidth,
              char inFixedWidth );
        
        ~Font();
        
        
        // draws a string on the current screen
        // returns x coordinate of string end
        int drawString( char *inString, int inX, int inY, rgbaColor inColor,
                         TextAlignment inAlign = alignCenter );


        
    private:
        
        // returns x coordinate to right of drawn character
        int drawCharacter( char inC, int inX, int inY, rgbaColor inColor );
        
        int measureString( char *inString );
        
        
        int mCharSpacing;
        int mSpaceWidth;
        
        char mFixedWidth;
        
        int mSpriteWidth;
        
        // maps ascii chars to sprite IDs
        int mSpriteMap[ 128 ];
        
        // for kerning (ignored if fixed width flag on)
        int mCharWidth[ 128 ];
        
    };

