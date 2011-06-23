#include "greenBarPaper.h"
#include "common.h"
#include "colors.h"


// topLeft, topRight, topFill,
// bottomLeft, bottomRight, bottomFill,
// leftFill,
// rightFill,
// centerFill
static int greenBarSprites[9];

static intPair greenBarPartOffsets[9] = 
  { {0,   0},   {240, 0},   {16, 0},
    {0,   176}, {240, 176}, {16, 176},
    {0,   32},
    {240, 32},
    {16,  16} };

static intPair greenBarPartSizes[9] = 
  { {16, 32}, {16, 32}, {16, 16},
    {16, 16}, {16, 16}, {16, 16},
    {16, 16},
    {16, 16},
    {16, 32} };


rgbaColor greenBarWhite = { 209, 209, 209, 255 };
rgbaColor greenBarGreen = { 162, 196, 172, 255 };




void initGreenBarPaper() {
    loadTiledSprites( "greenBarSheet.tga", 9,
                      greenBarSprites, 
                      greenBarPartOffsets, greenBarPartSizes, false );
    }



void freeGreenBarPaper() {
    }



// draws it across screen, starting at x=0
void drawGreenBarPaper( int inSheetTopY, int inBottomY ) {
    
    // every page has at least the top stuff
    for( int p=0; p<2; p++ ) {
        intPair offset = greenBarPartOffsets[p];

        drawSprite( greenBarSprites[p],
                    offset.x, inSheetTopY + offset.y, white );
        }
    
    // top fill
    intPair offset = greenBarPartOffsets[2];
    for( int f=0; f<14; f++ ) {
        drawSprite( greenBarSprites[2],
                    offset.x + 16  * f, inSheetTopY + offset.y, white );
        }

    // at least the first row, too
    
    // left fill
    offset = greenBarPartOffsets[6];

    drawSprite( greenBarSprites[6],
                offset.x, inSheetTopY + offset.y, white );

    // right fill
    offset = greenBarPartOffsets[7];

    drawSprite( greenBarSprites[7],
                offset.x, inSheetTopY + offset.y, white );

    
    // center fill
    offset = greenBarPartOffsets[8];
    offset.y += inSheetTopY;
    for( int f=0; f<14; f++ ) {
        drawSprite( greenBarSprites[8],
                    offset.x + 16  * f, offset.y, white );
        }


    
    int yCoverage = inSheetTopY + 48;
    
    
    int rowsDrawn = 0;

    while( yCoverage < inBottomY && rowsDrawn < 4 ) {
        
        // draw another row
        int rowYOffset = inSheetTopY + 48 + rowsDrawn * 32;
        
        
        // TWO rows of left fill
        offset = greenBarPartOffsets[6];

        drawSprite( greenBarSprites[6],
                    offset.x, 
                    rowYOffset, white );
        
        drawSprite( greenBarSprites[6],
                    offset.x, 
                    rowYOffset + 16, white );


        offset = greenBarPartOffsets[7];

        drawSprite( greenBarSprites[7],
                    offset.x, 
                    rowYOffset, white );
        
        drawSprite( greenBarSprites[7],
                    offset.x, 
                    rowYOffset + 16, white );
        

        
        // center fill
        offset = greenBarPartOffsets[8];
        
        for( int f=0; f<14; f++ ) {
            drawSprite( greenBarSprites[8],
                        offset.x + 16  * f, 
                        rowYOffset, white );
            }      

        yCoverage += 16;
        rowsDrawn ++;
        }
    
    

    if( yCoverage < inBottomY ) {
    
        // still more to fill after we've drawn max possible rows of
        // "center" stuff.

        // draw bottom of page
        

        for( int p=3; p<5; p++ ) {
            intPair offset = greenBarPartOffsets[p];
            
            drawSprite( greenBarSprites[p],
                        offset.x, inSheetTopY + offset.y, white );
            }
    
        // bottom fill
        intPair offset = greenBarPartOffsets[5];
        for( int f=0; f<14; f++ ) {
            drawSprite( greenBarSprites[5],
                        offset.x + 16  * f, inSheetTopY + offset.y, white );
            }
        
        }
    


    }









rgbaColor getGreenBarInkColor( int inFontY, int inMonthsLeft, 
                               char inSmallFont ) {
    
    rgbaColor paperColor = greenBarWhite;
    
    int lineNumber = inFontY / 16;
    
    if( lineNumber < 10 && lineNumber % 2 == 1 ) {
        // odd line, not one of last 2 lines
        paperColor = greenBarGreen;
        }
    
    
    rgbaColor baseInkColor = black;
    
    int maxBlackWeight = 192;
    int minBlackWeight = 64;
    
    if( inSmallFont ) {
        // don't let small font get as light
        minBlackWeight = 128;
        maxBlackWeight = 255;
        }
    
    int blackWeight = ( (maxBlackWeight - minBlackWeight) * inMonthsLeft / 8 ) 
        + minBlackWeight;
    

    return blendColors( baseInkColor, paperColor, (unsigned char)blackWeight );
    }


