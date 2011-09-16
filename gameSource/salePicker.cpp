#include "salePicker.h"
#include "common.h"
#include "platform.h"
#include "sprite.h"
#include "pause.h"


static int pickerSprite;
static int pickerW, pickerH;

// defined in bidPicker.cpp
extern int pickerBorderSprite;


static int sale;


static intPair lastCenter;


void initSalePicker() {
    pickerSprite = loadSprite( "salePicker.tga", &pickerW, &pickerH, true );
    
    }


void freeSalePicker() {
    //
    }



void setPickerSale( int inSale ) {
    sale = inSale;
    }



int getPickerSale() {
    return sale;
    }


static rgbaColor white = { 255, 255, 255, 255 };


void drawSalePicker( int inCenterX, int inCenterY ) {
    if( isPauseShowing() ) {
        return;
        }
    
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



void clickSalePicker( int inX, int inY ) {
    if( intAbs( inY - lastCenter.y ) > 28 ) {
        // off top or bottom by a large margin 
        return;
        }
    if( intAbs( inX - lastCenter.x ) > 12 ) {
        // off left or right by a large margin
        return;
        }


    // only consider y values, make it as easy to click as possible    
    if( lastCenter.y - inY > 2 ) {
        sale ++;
        }
    else if( inY - lastCenter.y > 0 ) {
        sale --;
        if( sale < 0 ) {
            sale = 0;
            }
        }
        
    }

