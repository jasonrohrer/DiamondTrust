#include "Button.h"

#include "platform.h"
#include "common.h"
#include "colors.h"
#include "minorGems/util/stringUtils.h"



static int buttonSpriteID;
static int buttonW, buttonH;
static int buttonDisplayW = 60;


static int longButtonSpriteID;
static int longButtonW, longButtonH;



void initButton() {
    
    buttonSpriteID = loadSprite( "button.tga", &buttonW, &buttonH, true );
    longButtonSpriteID = 
        loadSprite( "longButton.tga", &longButtonW, &longButtonH, true );
    }



Button::Button( Font *inFont, char *inText, int inX, int inY )
        : mFont( inFont ), mText( stringDuplicate( inText ) ), 
          mX( inX ), mY( inY ) {
    
    int textWidth = mFont->measureString( mText );

    mClickRadiusY = 13;
    
    if( textWidth > buttonDisplayW ) {
        mLong = true;
        mClickRadiusX = 64;
        }
    else {
        mLong = false;
        mClickRadiusX = 33;
        }
    
    mTextX = mX;

    mTextY = mY - 6;
    }



Button::~Button() {
    delete [] mText;
    }


char Button::getPressed( int inClickX, int inClickY ) {
    if( inClickY > mY - mClickRadiusY && inClickY < mY + mClickRadiusY
        &&
        inClickX > mX - mClickRadiusX && inClickX < mX + mClickRadiusX ) {
        
        return true;
        }

    return false;
    }


void Button::draw() {
    int spriteID = buttonSpriteID;
    int w = buttonW;
    if( mLong ) {
        spriteID = longButtonSpriteID;
        w = longButtonW;
        } 

    drawSprite( spriteID, mX - w/2, mY - buttonH/2, white );
    
    startNewSpriteLayer();

    mFont->drawString( mText, mTextX, mTextY, black, alignCenter );
    }
