
#include "bidPicker.h"
#include "common.h"
#include "platform.h"


static int pickerSprite;
static int pickerW, pickerH;

static int bid;

static char done;

intPair lastCenter;


void initBidPicker() {
    pickerSprite = loadSprite( "bidPicker.tga", &pickerW, &pickerH, true );    
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

    if( inCenterY + 21 > 191 ) {
        // off bottom, adjust
        inCenterY = 191 - 21;
        }
    else if( inCenterY - 21 < 0 ) {
        // off top, adjust
        inCenterY = 21;
        }
    
    
    drawSprite( pickerSprite, inCenterX - pickerW/2, inCenterY - pickerH/2,
                white );

    lastCenter.x = inCenterX;
    lastCenter.y = inCenterY;
    }



void clickBidPicker( int inX, int inY ) {
    
    if( intAbs( inY - lastCenter.y ) > 22 ) {
        // off top or bottom
        return;
        }
    if( intAbs( inX - lastCenter.x ) > 8 ) {
        // off left or right
        return;
        }
    
    intPair upCenter = lastCenter;
    intPair doneCenter = lastCenter;
    intPair downCenter = lastCenter;
    
    upCenter.y -= 14;
    downCenter.y += 14;
    
    intPair click = { inX, inY };
    
    if( intDistance( click, upCenter ) < 6 ) {
        bid ++;
        }
    else if( intDistance( click, downCenter ) < 6 ) {
        bid --;
        if( bid < 0 ) {
            bid = 0;
            }
        }
    else if( intDistance( click, doneCenter ) < 6 ) {    
        done = true;
        }
    }


char isBidDone() {
    return done;
    }

