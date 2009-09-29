#include "salePicker.h"
#include "common.h"
#include "platform.h"
#include "sprite.h"


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

    if( inCenterY + 21 > 191 ) {
        // off bottom, adjust
        inCenterY = 191 - 21;
        }
    else if( inCenterY - 21 < 0 ) {
        // off top, adjust
        inCenterY = 21;
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
    
    if( intAbs( inY - lastCenter.y ) > 22 ) {
        // off top or bottom
        return;
        }
    if( intAbs( inX - lastCenter.x ) > 8 ) {
        // off left or right
        return;
        }
    
    intPair upCenter = lastCenter;
    intPair downCenter = lastCenter;
    
    upCenter.y -= 14;
    downCenter.y += 14;
    
    intPair click = { inX, inY };
    
    if( intDistance( click, upCenter ) < 6 ) {
        sale ++;
        }
    else if( intDistance( click, downCenter ) < 6 ) {
        sale --;
        if( sale < 0 ) {
            sale = 0;
            }
        }
    }

