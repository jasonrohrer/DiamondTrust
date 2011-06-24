#include "Button.h"

#include "platform.h"
#include "common.h"
#include "colors.h"
#include "minorGems/util/stringUtils.h"



static int buttonSpriteID;
static int buttonW, buttonH;
// leave 2-pixel border on each side
static int buttonDisplayW = 56 - 4;


static int longButtonSpriteID;
static int longButtonW, longButtonH;
// leave 2-pixel border on each side
static int longButtonDisplayW = 118 - 4;

// variable-length button
static int buttonLeftEndSpriteID;
static int buttonMiddleSpriteID;
static int buttonRightEndSpriteID;
static int buttonPartW, buttonPartH;



void initButton() {
    
    buttonSpriteID = loadSprite( "button.tga", &buttonW, &buttonH, true );
    longButtonSpriteID = 
        loadSprite( "longButton.tga", &longButtonW, &longButtonH, true );
    

    buttonLeftEndSpriteID = 
        loadSprite( "buttonLeftEnd.tga", &buttonPartW, &buttonPartH, true );
   
    buttonMiddleSpriteID = 
        loadSprite( "buttonMiddle.tga", &buttonPartW, &buttonPartH, true );
    
    buttonRightEndSpriteID = 
        loadSprite( "buttonRightEnd.tga", &buttonPartW, &buttonPartH, true );
    
    }



Button::Button( Font *inFont, char *inText, int inX, int inY )
        : mFont( inFont ), mText( stringDuplicate( inText ) ), 
          mX( inX ), mY( inY ) {
    
    int textWidth = mFont->measureString( mText );

    mClickRadiusY = 13;
    
    mNumMiddleParts = -1;

    if( textWidth <= buttonDisplayW ) {
        mLong = false;
        mClickRadiusX = 33;

        mW = buttonW;
        mH = buttonH;
        }
    else if( textWidth <= longButtonDisplayW ) {
        mLong = true;
        mClickRadiusX = 64;

        mW = longButtonW;
        mH = longButtonH;
        }
    else {
        mLong = false;
        
        mNumMiddleParts = 0;
        
        while( mNumMiddleParts * buttonPartW < textWidth ) {
            mNumMiddleParts ++;
            }
        
        mW = ( mNumMiddleParts + 2 ) * buttonPartW;
        mH = buttonPartH;
        
        mClickRadiusX = mW / 2;
        }
    
            
    mTextX = mX;

    mTextY = mY - 6;
    }



Button::~Button() {
    delete [] mText;
    }



int Button::getWidth() {
    return mW;
    }



int Button::getCenterX() {
    return mX;
    }

int Button::getCenterY() {
    return mY;
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

    if( mNumMiddleParts < 0 ) {
        
        int spriteID = buttonSpriteID;

        if( mLong ) {
            spriteID = longButtonSpriteID;
            }

        drawSprite( spriteID, mX - mW/2, mY - mH/2, white );
        }
    else {
        // variable length button
        
        int y = mY - mH/2;;

        int x = mX - mW/2;
        
        drawSprite( buttonLeftEndSpriteID, x, y, white );

        for( int i=0; i<mNumMiddleParts; i++ ) {
            x += buttonPartW;
            
            drawSprite( buttonMiddleSpriteID, x, y, white );
            }

        x += buttonPartW;
        drawSprite( buttonRightEndSpriteID, x, y, white );    
        }
    
    startNewSpriteLayer();

    mFont->drawString( mText, mTextX, mTextY, buttonInkColor, alignCenter );
    }
