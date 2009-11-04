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

    if( textWidth > buttonDisplayW ) {
        mLong = true;
        }
    else {
        mLong = false;
        }
    
    if( !mLong ) {
        mTextX = mX + buttonW / 2;
        }
    else {
        mTextX = mX + longButtonW / 2;
        }
    
    mTextY = mY + 10;
    }



Button::~Button() {
    delete [] mText;
    }


char Button::getPressed( int inClickX, int inClickY ) {
    if( !mLong ) {    
        if( inClickY > mY && inClickY < mY + buttonH
            &&
            inClickX > mX && inClickX < mX + buttonDisplayW ) {
            
            return true;
            }
        }
    else {
        if( inClickY > mY && inClickY < mY + longButtonH
            &&
            inClickX > mX && inClickX < mX + longButtonW ) {
            
            return true;
            }
        }
    

    return false;
    }


void Button::draw() {
    int spriteID = buttonSpriteID;
    if( mLong ) {
        spriteID = longButtonSpriteID;
        } 

    drawSprite( spriteID, mX, mY, white );
    
    startNewSpriteLayer();

    mFont->drawString( mText, mTextX, mTextY, black, alignCenter );
    }
