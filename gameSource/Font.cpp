#include "Font.h"
#include "tga.h"

#include <string.h>



Font::Font( char *inFileName, int inCharSpacing, int inSpaceWidth,
            char inFixedWidth )
        : mCharSpacing( inCharSpacing ), mSpaceWidth( inSpaceWidth ),
          mFixedWidth( inFixedWidth ) {
    
    int fileDataSize;
    unsigned char *spriteFileData = readFile( inFileName, 
                                              &fileDataSize );
    if( spriteFileData != NULL ) {
        
        int width, height;
        
        rgbaColor *spriteRGBA = extractTGAData( spriteFileData, fileDataSize,
                                                &width, &height );
        
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
            
                        
                
            mSpriteWidth = width / 16;
            
            int pixelsPerChar = mSpriteWidth * mSpriteWidth;
            
            for( int i=0; i<128; i++ ) {
                int yOffset = ( i / 16 ) * mSpriteWidth;
                int xOffset = ( i % 16 ) * mSpriteWidth;
                
                rgbaColor *charRGBA = new rgbaColor[ pixelsPerChar ];
                
                for( int y=0; y<mSpriteWidth; y++ ) {
                    for( int x=0; x<mSpriteWidth; x++ ) {
                        
                        int imageIndex = (y + yOffset) * width
                            + x + xOffset;
                        int charIndex = y * mSpriteWidth + x;
                        
                        charRGBA[ charIndex ] = spriteRGBA[ imageIndex ];
                        }
                    }
                
                
                mSpriteMap[i] = 
                    addSprite( charRGBA, mSpriteWidth, mSpriteWidth );
                

                if( mFixedWidth ) {
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
                            
                            unsigned char r = 
                                charRGBA[ y * mSpriteWidth + x ].r;
                            
                            if( r > 0 ) {
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
                    }
                

                delete [] charRGBA;
                }
                        
            delete [] spriteRGBA;
            }

        delete [] spriteFileData;
        }
    }



Font::~Font() {
    //
    }



int Font::drawString( char *inString, int inX, int inY, rgbaColor inColor,
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
        int charWidth = drawCharacter( inString[i], x, inY, inColor );
        x += charWidth + mCharSpacing;
        }
    // no spacing after last character
    x -= mCharSpacing;

    return x;
    }



int Font::drawCharacter( char inC, int inX, int inY, rgbaColor inColor ) {
    if( inC == ' ' ) {
        return mSpaceWidth;
        }

    int charStartX = inX;
    
    if( !mFixedWidth ) {
        charStartX -= mCharLeftEdgeOffset[ (int)inC ];
        }
    
    
    drawSprite( mSpriteMap[ (int)inC ], charStartX, inY, inColor );
    
    
    if( mFixedWidth ) {
        return mSpriteWidth;
        }
    else {
        return mCharWidth[ (int)inC ];
        }
    }



int Font::measureString( char *inString ) {
    unsigned int numChars = strlen( inString );

    int width = 0;
    
    for( unsigned int i=0; i<numChars; i++ ) {
        char c = inString[i];
        
        if( c == ' ' ) {
            width += mSpaceWidth;
            }
        else if( mFixedWidth ) {
            width += mSpriteWidth;
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
