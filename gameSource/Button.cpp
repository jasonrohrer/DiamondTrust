#include "Button.h"

#include "platform.h"
#include "common.h"
#include "minorGems/util/stringUtils.h"



static int buttonSpriteID;
static int buttonW, buttonH;

static rgbaColor white = { 255, 255, 255, 255 };


void initButton() {
    
    buttonSpriteID = loadSprite( "button.tga", &buttonW, &buttonH, true );
    }



Button::Button( Font *inFont, char *inText, int inX, int inY )
        : mFont( inFont ), mText( stringDuplicate( inText ) ), 
          mX( inX ), mY( inY ) {
    
    mTextX = mX + buttonW / 2;
    mTextY = mY + 2;
    }



Button::~Button() {
    delete [] mText;
    }


char Button::getPressed( int inClickX, int inClickY ) {
    if( inClickY > mY && inClickY < mY + buttonH
        &&
        inClickX > mX && inClickX < mX + buttonW ) {
        
        return true;
        }
    return false;
    }


void Button::draw() {
    drawSprite( buttonSpriteID, mX, mY, white );
    
    startNewSpriteLayer();
    
    mFont->drawString( mText, mTextX, mTextY, white, alignCenter );
    };
