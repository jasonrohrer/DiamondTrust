#include "Font.h"
#include "tga.h"

#include <string.h>



Font::Font( const char *inFileName, int inCharSpacing, int inSpaceWidth,
            char inFixedWidth, RGBAFilter *inFilter  )
        : mCharSpacing( inCharSpacing ), mSpaceWidth( inSpaceWidth ),
          mFixedWidth( inFixedWidth ), mRaiseDollarSign( false ) {
    

    int width, height;

    rgbaColor *spriteRGBA = readTGAFile( inFileName, &width, &height );

    if( spriteRGBA != NULL ) {
                        
        // use corner color as transparency
        // copy red channel into other channels
        // (ignore other channels, even for transparency)

        spriteRGBA[0].a = 0;
        unsigned char tr;
        tr = spriteRGBA[0].r;

        int numPixels = width * height; 
        for( int i=0; i<numPixels; i++ ) {
            unsigned char r = spriteRGBA[i].r;
                
            if( r == tr ) {
                // matches corner r
                spriteRGBA[i].a = 0;
                }
                
            spriteRGBA[i].g = r;
            spriteRGBA[i].b = r;
            }
        
        if( inFilter != NULL ) {
            inFilter->filter( spriteRGBA, width, height );
            }
        
                        
                
        mSpriteWidth = width / 16;
            
        int pixelsPerChar = mSpriteWidth * mSpriteWidth;
            
        rgbaColor *charRGBA = new rgbaColor[ pixelsPerChar ];
        
        int maxNumeralWidth = 0;

        for( int i=0; i<128; i++ ) {
            int yOffset = ( i / 16 ) * mSpriteWidth;
            int xOffset = ( i % 16 ) * mSpriteWidth;
                                
            for( int y=0; y<mSpriteWidth; y++ ) {
                for( int x=0; x<mSpriteWidth; x++ ) {
                        
                    int imageIndex = (y + yOffset) * width
                        + x + xOffset;
                    int charIndex = y * mSpriteWidth + x;
                        
                    charRGBA[ charIndex ] = spriteRGBA[ imageIndex ];
                    }
                }
                
            // don't bother consuming texture ram for blank sprites
            char allTransparent = true;
                
            for( int p=0; p<pixelsPerChar && allTransparent; p++ ) {
                if( charRGBA[ p ].a != 0 ) {
                    allTransparent = false;
                    }
                }
                
            if( !allTransparent ) {
                mSpriteMap[i] = 
                    addSprite( charRGBA, mSpriteWidth, mSpriteWidth );
                }
            else {
                mSpriteMap[i] = -1;
                }
                

            if( mFixedWidth ) {
                mCharLeftEdgeOffset[i] = 0;
                mCharWidth[i] = mSpriteWidth;
                }
            else if( allTransparent ) {
                mCharLeftEdgeOffset[i] = 0;
                mCharWidth[i] = mSpriteWidth;
                }
            else {
                // implement pseudo-kerning
                    
                int farthestLeft = mSpriteWidth;
                int farthestRight = 0;
                    
                char someInk = false;
                    
                for( int y=0; y<mSpriteWidth; y++ ) {
                    for( int x=0; x<mSpriteWidth; x++ ) {
                            
                        unsigned char a = 
                            charRGBA[ y * mSpriteWidth + x ].a;
                            
                        if( a > 0 ) {
                            someInk = true;

                            if( x < farthestLeft ) {
                                farthestLeft = x;
                                }
                            if( x > farthestRight ) {
                                farthestRight = x;
                                }
                            }
                        }
                    }
                    
                if( ! someInk  ) {
                    mCharLeftEdgeOffset[i] = 0;
                    mCharWidth[i] = mSpriteWidth;
                    }
                else {
                    mCharLeftEdgeOffset[i] = farthestLeft;
                    mCharWidth[i] = farthestRight - farthestLeft + 1;

                    
                    }

                if( i >= '0' && i <= '9' ) {
                    // a numeral
                    if( mCharWidth[i] > maxNumeralWidth ) {
                        maxNumeralWidth = mCharWidth[i];
                        }
                    }
                }
                
            
            }

        // make sure all numerals are same width (same as widest numeral)
        for( int i='0'; i<='9'; i++ ) {
            if( mCharWidth[i] < maxNumeralWidth ) {
                // pad on LEFT side to keep compact spacing for 2-digit
                // numbers
                
                int extra = maxNumeralWidth - mCharWidth[i];

                if( extra < 2 ) {
                    mCharLeftEdgeOffset[i] -= extra;
                    }
                else {
                    // try to split up extra to keep character centered
                    mCharLeftEdgeOffset[i] -= extra / 2;
                    }
                mCharWidth[i] = maxNumeralWidth;
                }
            
            }

        delete [] charRGBA;
                        
        delete [] spriteRGBA;
        }
    

    }



Font::~Font() {
    //
    }



void Font::setRaiseDollarSign( char inRaise ) {
    mRaiseDollarSign = inRaise;
    }




int Font::drawString( const char *inString, int inX, int inY, rgbaColor inColor,
                      TextAlignment inAlign ) {
    unsigned int numChars = strlen( inString );
    
    int x = inX;
    
    int stringWidth = 0;
    
    if( inAlign != alignLeft ) {
        stringWidth = measureString( inString );
        }
    
    switch( inAlign ) {
        case alignCenter:
            x -= stringWidth / 2;
            break;
        case alignRight:
            x -= stringWidth;
            break;
        default:
            // left?  do nothing
            break;            
        }
    
    for( unsigned int i=0; i<numChars; i++ ) {
        char c = inString[i];

        // handle characters that are out of ASCII range
        if( c < 0 ) {
            c = '?';
            }
        
        
        if( c == ' ' ) {
            x += mSpaceWidth;
            }
        else {
            
            int yOffset = 0;
            
            if( c == '$' && mRaiseDollarSign ) {
                yOffset = -1;
                }
            

            int charInt = (int)c;

            int spriteID = mSpriteMap[ charInt ];
            
            if( spriteID != -1 ) {
                drawSprite( spriteID, 
                            x - mCharLeftEdgeOffset[ charInt ],
                            inY + yOffset, 
                            inColor );
                }
    
            x += mCharWidth[ charInt ];
            }
        
        x += mCharSpacing;
        }
    // no spacing after last character
    x -= mCharSpacing;

    return x;
    }



int Font::measureString( const char *inString ) {
    unsigned int numChars = strlen( inString );

    int width = 0;
    
    for( unsigned int i=0; i<numChars; i++ ) {
        char c = inString[i];
        
        if( c == ' ' ) {
            width += mSpaceWidth;
            }
        else {
            width += mCharWidth[ (int)c ];
            }
    
        width += mCharSpacing;
        }

    // no extra space at end 
    width -= mCharSpacing;
    
    return width;
    }




int Font::getCharacterSpacing() {
    return mCharSpacing;
    }

