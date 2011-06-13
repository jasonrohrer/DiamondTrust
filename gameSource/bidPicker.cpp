
#include "bidPicker.h"
#include "common.h"
#include "platform.h"
#include "sprite.h"


static int pickerSprite;
static int pickerW, pickerH;

int pickerBorderSprite;


static int bid;

static char done;

static intPair lastCenter;


void initBidPicker() {
    pickerSprite = loadSprite( "bidPicker.tga", &pickerW, &pickerH, true );    
    pickerBorderSprite = loadSprite( "pickerBorder.tga", true );    
    }


void freeBidPicker() {
    //
    }



void setPickerBid( int inBid ) {
    bid = inBid;
    done = false;
    }



int getPickerBid() {
    return bid;
    }


static rgbaColor white = { 255, 255, 255, 255 };


void drawBidPicker( int inCenterX, int inCenterY ) {

    // avoid drawing too close to edge (avoid putting controls in 8-dot
    // border region of screen)
    if( inCenterY + 27 > 191 ) {
        // off bottom, adjust
        inCenterY = 191 - 27;
        }
    else if( inCenterY - 28 < 0 ) {
        // off top, adjust
        inCenterY = 28;
        }
    
    
    drawSprite( pickerSprite, 
                inCenterX - pickerW/2, inCenterY - pickerH/2,
                white );

    startNewSpriteLayer();

    drawBlinkingSprite( pickerBorderSprite, 
                        inCenterX - pickerW/2, inCenterY - pickerH/2,
                        white );

    lastCenter.x = inCenterX;
    lastCenter.y = inCenterY;
    }



char clickBidPicker( int inX, int inY ) {
    
    if( intAbs( inY - lastCenter.y ) > 28 ) {
        // off top or bottom by a large margin 
        return false;
        }
    if( intAbs( inX - lastCenter.x ) > 12 ) {
        // off left or right by a large margin
        return false;
        }
    
    // only consider y values, make it as easy to click as possible    
    if( lastCenter.y - inY > 8 ) {
        bid ++;
        }
    else if( inY - lastCenter.y > 8 ) {
        bid --;
        if( bid < 0 ) {
            bid = 0;
            }
        }
    else {
        done = true;
        }

    return true;
    }


char isBidDone() {
    return done;
    }

