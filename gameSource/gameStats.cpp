#include "gameStats.h"
#include "platform.h"
#include "common.h"
#include "Font.h"
#include "colors.h"
#include "units.h"
#include "map.h"
#include "tga.h"
#include "music.h"

#include "minorGems/util/stringUtils.h"


extern char pictureSendSpriteID;
extern char pictureSendSpriteSet;



static int monthsLeft = 12;

static int money[2] = {0,0};
static int diamonds[2] = {0,0};
static int selling[2] = {0,0};

static char opponentMoneyUnknown = false;

static char showInspectorPanelFlag = false;



static char showSaleFlag = false;
static char peekSaleFlag = false;
static char revealSaleFlag = false;

static char *sellZeroNote = NULL;
static char *sellSomeNote = NULL;


void setMonthsLeft( int inMonths ) {
    monthsLeft = inMonths;
    }

int getMonthsLeft() {
    return monthsLeft;
    }

void decrementMonthsLeft() {
    monthsLeft--;
    if( monthsLeft < 0 ) {
        monthsLeft = 0;
        }

    switch( monthsLeft ) {
        case 5:
            nextSongAct();
            break;
        case 2:
            nextSongAct();
            break;
        };
    }





int getPlayerMoney( int inPlayer ) {
    return money[ inPlayer ];
    }

int getPlayerDiamonds( int inPlayer ) {
    return diamonds[inPlayer];
    }


void setPlayerMoney( int inPlayer, int inMoney ) {
    money[inPlayer] = inMoney;
    }


void setPlayerDiamonds( int inPlayer, int inDiamonds ) {
    diamonds[inPlayer] = inDiamonds;
    }

void addPlayerMoney( int inPlayer, int inMoney ) {
    money[inPlayer] += inMoney;
    }


void addPlayerDiamonds( int inPlayer, int inDiamonds ) {
    diamonds[inPlayer] += inDiamonds;
    }


void setOpponentMoneyUnknown( char inUnknown ) {
    opponentMoneyUnknown = inUnknown;
    }


void showInspectorPanel( char inShow ) {
    showInspectorPanelFlag = inShow;
    }





static int statusPanelSprite;
static int panelW, panelH;

static int datePanelSprite;
static int datePanelW, datePanelH;

// broken up into parts to save space
// left, middle, right, fill-column
static int sellingPanelSprite[4];
static intPair sellingPanelPartOffsets[4] = 
  { {0,0}, {72,0}, {120,0}, {8,0} };
static intPair sellingPanelPartSizes[4] = 
  { {8,64}, {16,64}, {8,64}, {8,64} };
//static int sellingPanelW, sellingPanelH;




// broken up into parts to save space
// top, bottom, left, right, fill-block
static int unitInfoPanelSprite[5];
static intPair unitInfoPanelPartOffsets[5] = 
  { {0,0}, {0,64}, {0,0}, {248,0}, {8,8} };
static intPair unitInfoPanelPartSizes[5] = 
  { {256,8}, {256,64}, {8,128}, {8,128}, {8,8} };
//static int unitPanelW, unitPanelH;

// color to use for paper-based fonts that are on top of rolodex card
static rgbaColor unitInfoPanelPaperColor = { 223, 223, 167, 255 };



static int pictureSprites[2][3];
static int pictureSpriteW, pictureSpriteH;



static void loadMultiSprite( const char *inFileName, int inNumFrames,
                             int *outSpriteIDs, int *outW, int *outH,
                             int inSetID = -1 ) {

    int w, h;
    
    rgbaColor *spriteRGBA = readTGAFile( inFileName,
                                         &w, &h );
    
    
    if( spriteRGBA == NULL ) {
        printOut( "Reading multisprite file %s failed.\n", inFileName );
        return;
        }


    // 1 pixel row between each sprite image
    int spriteH = ((h + 1) /  inNumFrames ) - 1;
    
    for( int f=0; f<inNumFrames; f++ ) {
        
        rgbaColor *frameSubImage = 
            &( spriteRGBA[ f * (spriteH + 1) * w ] );
        
        int numPixels = spriteH * w;
        
        applyCornerTransparency( frameSubImage, numPixels );

        outSpriteIDs[f] = 
            addSprite( frameSubImage, w, spriteH, inSetID );
        }

    delete [] spriteRGBA;

    *outW = pictureSpriteW;
    *outH = pictureSpriteH;
    }



void initStats() {
    statusPanelSprite = loadSprite( "statsPanel.tga", 
                                    &panelW, &panelH, true );
    datePanelSprite = loadSprite( "datePanel.tga", 
                                  &datePanelW, &datePanelH, true );
    /*
    sellingPanelSprite = loadSprite( "sellingPanel.tga", 
                                     &sellingPanelW, &sellingPanelH, true );
    */

    loadTiledSprites( "sellingPanel.tga", 4,
                      sellingPanelSprite, 
                      sellingPanelPartOffsets, sellingPanelPartSizes, true );
    
    /*
      unitInfoPanelSprite = loadSprite( "rolodex_manilla4.tga", 
      &unitPanelW, &unitPanelH, true );
    */

    
    loadTiledSprites( "rolodex_manilla16.tga", 5,
                      unitInfoPanelSprite, 
                      unitInfoPanelPartOffsets, unitInfoPanelPartSizes, true );
    /*
    rgbaColor *unitInfoPanelRGBA = readTGAFile( "rolodex_manilla4.tga",
                                                &unitPanelW, &unitPanelH );

    if( unitInfoPanelRGBA == NULL ) {
        
        printOut( "Reading unitInfoPanel sprite failed.\n" );
        return;
        }
    applyCornerTransparency( unitInfoPanelRGBA, unitPanelW * unitPanelH );
    
    
    for( int p=0; p<5; p++ ) {
        intPair offset = unitInfoPanelPartOffsets[p];
        intPair size = unitInfoPanelPartSizes[p];
        
        printOut( "Extracting region at %d,%d of size %d,%d\n",
                  offset.x, offset.y,
                  size.x, size.y );
        
        rgbaColor *partRGBA = extractRegion( unitInfoPanelRGBA, 
                                             unitPanelW, unitPanelH,
                                             offset.x, offset.y,
                                             size.x, size.y );
        unitInfoPanelSprite[p] = addSprite( partRGBA, size.x, size.y );
    
        delete [] partRGBA;
        }
        
    delete [] unitInfoPanelRGBA;
    */

    int pictureSpriteSet = createSpriteSet();

    loadMultiSprite( "playerPictures.tga", 3,
                     pictureSprites[0], &pictureSpriteW, &pictureSpriteH,
                     pictureSpriteSet );
    loadMultiSprite( "enemyPictures.tga", 3,
                     pictureSprites[1], &pictureSpriteW, &pictureSpriteH,
                     pictureSpriteSet );

    sellZeroNote = autoSprintf( translate( "stats_sellZeroNote" ),
                                noSaleFlatRate );
    sellSomeNote = autoSprintf( translate( "stats_sellSomeNote" ),
                                saleNet );

    }


void freeStats() {
    if( sellZeroNote != NULL ) {
        delete [] sellZeroNote;
        sellZeroNote = NULL;
        }
    if( sellSomeNote != NULL ) {
        delete [] sellSomeNote;
        sellSomeNote = NULL;
        }
    
    }




void resetStats() {
    setPlayerMoney( 0, 18 );
    setPlayerMoney( 1, 18 );    

    setPlayerDiamonds( 0, 0 );
    setPlayerDiamonds( 1, 0 );    

    // FIXME:  testing
    //setPlayerDiamonds( 0, 10 );
    //setPlayerDiamonds( 1, 10 );    

    // actually, it makes sense for players to start the game with a few
    // diamonds (so that the SellDiamonds phase has an interesting decision
    // right off the bat).  2 each allows full exploration of payoff matrix.
    setPlayerDiamonds( 0, 2 );
    setPlayerDiamonds( 1, 2 );    


    // show opponent's money at start of game
    setOpponentMoneyUnknown( false );
    
    setMonthsLeft( 8 );
    // for testing
    //setMonthsLeft( 0 );

    backToFirstSongAct();
    }




void showSale( char inShow ) {
    showSaleFlag = inShow;
    if( showSaleFlag == true ) {
        peekSaleFlag = false;
        revealSaleFlag = false;
        }
    }


void setPlayerNumToSell( int inPlayer, int inNumToSell ) {
    selling[ inPlayer ] = inNumToSell;
    }


int getNumSold( int inPlayer ) {
    return selling[ inPlayer ];
    }


void peekSale() {
    peekSaleFlag = true;
    }


void finishSale() {
    revealSaleFlag = true;
    }


int getPlayerEarnings( int inPlayer ) {
    if( selling[inPlayer] == 0 ) {
        return noSaleFlatRate;
        }
    
    return ( saleNet * selling[inPlayer] ) / ( selling[0] + selling[1] );
    }





static rgbaColor panelColors[3] = { playerPanelColor, enemyPanelColor,
                                    inspectorPanelColor };


extern Font *font16;
extern Font *font16Hand;
extern Font *font8;


// from map.cpp
extern int diamondSpriteID;
extern int diamondSpriteW;
extern int diamondSpriteH;


static void drawDiamondCounter( int inX, int inY, int inCount ) {
    
    
    
    drawSprite( diamondSpriteID, 
                inX - diamondSpriteW/2, 
                inY - diamondSpriteH/2, 
                white );

    startNewSpriteLayer();
    
    char *countString = autoSprintf( "%d", inCount );
    
    font16->drawString( countString, 
                        inX, 
                        inY - 7,
                        diamondPurple, 
                        alignCenter );

    delete [] countString;
    }



static void drawMoneyValue16( int inX, int inY, int inValue, 
                            rgbaColor inColor, char inValueHidden ) {

    font16Hand->drawString( "$", inX, inY - 1, inColor, alignLeft );
    
    char *moneyString;
        
    if( inValueHidden ) {
        moneyString = autoSprintf( "?" );
        }
    else {
        moneyString = autoSprintf( "%d", inValue );
        }
    
    // assume two digits wide, max, with right-aligned digits and fixed
    // "$" position

    // hackish fix if 3+ digits are required (violate right-alignment)

    // even more hackish:  there seems to be room for a 1 to sneak in there
    // for $100+ values
    // but probably not a 2 for $200+ values
    unsigned int numDigits = strlen( moneyString );
    if( numDigits > 2 && inValue > 199 ) {
        inX += (int)( 11 * (numDigits - 2) + 2 * (numDigits - 2) );
        }
    
    
    font16Hand->drawString( moneyString, 
                        inX + 37, 
                        inY,
                        inColor, 
                        alignRight );

    delete [] moneyString;
    }



static void drawMoneyValue8( int inX, int inY, int inValue, 
                             rgbaColor inColor, char inValueHidden ) {
    
    font8->drawString( "$", inX, inY - 1, inColor, alignLeft );
    
    char *moneyString;
        
    if( inValueHidden ) {
        moneyString = autoSprintf( "?" );
        }
    else {
        moneyString = autoSprintf( "%d", inValue );
        }
    
    // assume two digits wide, max, with right-aligned digits and fixed
    // "$" position

    // hackish fix if 3+ digits are required (violate right-alignment)
    unsigned int numDigits = strlen( moneyString );
    if( numDigits > 2 ) {
        inX += (int)( 5 * (numDigits - 2) + 1 * (numDigits - 2) );
        }
    
    
    font8->drawString( moneyString, 
                       inX + 17, 
                       inY,
                       inColor, 
                       alignRight );

    delete [] moneyString;
    }


char isOpponentMoneyVisible() {
    
    if( opponentMoneyUnknown ) {
        // show their money only if a bribed unit is at home
        for( int i=numPlayerUnits; i<numPlayerUnits*2; i++ ) {
            Unit *u = getUnit( i );
            if( getUnitRegion( i ) == 1 ) {
                // home
                
                if( u->mTotalBribe > u->mTotalSalary ) {
                    return true;
                    }
                }
            }
        
        
        return false;
        }
    else {
        return true;
        }
    }


static void drawPanelContents( int inX, int inPlayer ) {
    
    int baseY = 0;
    
    inX += 10;


    int showMoney = true;
    
    if( inPlayer == 1 ) {
        // other player
        
        showMoney = isOpponentMoneyVisible();
        }
    
            
    
    drawMoneyValue16( inX, baseY + 12, money[ inPlayer ], 
                      panelColors[inPlayer], !showMoney );

    inX += 64;
    drawDiamondCounter( inX, baseY + panelH / 2 + 3, diamonds[ inPlayer ] );
    }




static void drawSellStats( int inX, int inY, int inPlayer ) {
    
    int y = inY;
    

    // drawSprite( sellingPanelSprite, inX, y, panelColors[ inPlayer ] );
    

    for( int p=0; p<3; p++ ) {
        intPair offset = sellingPanelPartOffsets[p];

        drawSprite( sellingPanelSprite[p], 
                    inX + offset.x, y + offset.y, panelColors[ inPlayer ] );
        }
    // 3 is fill block... fill 8x1 grid of blocks starting at 8
    int blockID = sellingPanelSprite[3];
    intPair offset = sellingPanelPartOffsets[3];

    int xOffset = inX + offset.x;

    int x;
    for( x=0; x<8; x++ ) {
        drawSprite( blockID, 
                    xOffset, y, panelColors[ inPlayer ] );
        xOffset += offset.x;
        }
    
    // then fill 4x1 grid of blocks starting at 88
    xOffset = inX + 88;
    
    for( int x=0; x<4; x++ ) {
        drawSprite( blockID, 
                    xOffset, y, panelColors[ inPlayer ] );
        xOffset += offset.x;
        }




    startNewSpriteLayer();
    
    inX += 8;
    

    y += 17;


    // because fonts can vary in width (just switched to handwriting, which
    // is narrower), fix the xOffset to a good value (to put diamond sell count
    // and money earned in the next column of the ledger) instead of basing
    // it on font measurements
    xOffset = 85;
    

    font16Hand->drawString( translate( "stats_selling" ), 
                            inX, 
                            y - 8,
                            panelColors[inPlayer], 
                            alignLeft );
    
    
    drawDiamondCounter( inX + xOffset + 8, y - 1, selling[inPlayer] );

    y += 12;
    font16Hand->drawString( translate( "stats_earning" ), 
                            inX, 
                            y,
                            panelColors[inPlayer], 
                            alignLeft );
    
    
    
    if( selling[inPlayer] == 0 ) {
        drawMoneyValue16( inX + xOffset - 9, y, noSaleFlatRate, 
                          panelColors[inPlayer], false );
        }
    else {

        drawMoneyValue16( inX + xOffset - 9, y, 
                          getPlayerEarnings( inPlayer ), 
                          panelColors[inPlayer], 
                          !revealSaleFlag );
        }
    }


static void drawInfoPanelBackground( int inX, int inY, rgbaColor inC ) {
    for( int p=0; p<4; p++ ) {
        intPair offset = unitInfoPanelPartOffsets[p];

        drawSprite( unitInfoPanelSprite[p], 
                    inX + offset.x, inY + offset.y, inC );
        }
    // 4 is fill block... fill 30x7 grid of blocks
    int blockID = unitInfoPanelSprite[4];
    intPair offset = unitInfoPanelPartOffsets[4];

    int xOffset = inX + offset.x;
    
    int spriteX[30*7];
    int spriteY[30*7];

    for( int x=0; x<30; x++ ) {
        
        int yOffset = inY + offset.y;
        
        for( int y=0; y<7; y++ ) {
            spriteX[ x * 7 + y ] = xOffset;
            spriteY[ x * 7 + y ] = yOffset;

            yOffset += offset.y;
            }
        xOffset += offset.x;
        }

    drawSprite( blockID, 30 * 7, spriteX, spriteY, inC );
    }



// draws stats onto upper screen
void drawStats() {
    drawSprite( statusPanelSprite, 0, 0, panelColors[0] );
    
    int secondPanelX = 255 - 100;
    
    drawSprite( statusPanelSprite, secondPanelX, 0, panelColors[1] );


    drawSprite( datePanelSprite, 128 - datePanelW / 2, 0, white );
    

    startNewSpriteLayer();

    drawPanelContents( 0, 0 );
    drawPanelContents( secondPanelX, 1 );


    char *monthString = autoSprintf( "%d", monthsLeft );
    
    font16->drawString( monthString, 
                        128, 
                        3,
                        black, 
                        alignCenter );
    delete [] monthString;
    
    if( monthsLeft == 1 ) {
        // singular
        font8->drawString( translate( "stats_monthsLeft_A" ), 
                           128, 
                           16,
                           black, 
                           alignCenter );
        }
    else {
        // plural
        font8->drawString( translate( "stats_monthsLeft_As" ), 
                           128, 
                           16,
                           black, 
                           alignCenter );
        }
    
    font8->drawString( translate( "stats_monthsLeft_B" ), 
                        128, 
                        24,
                        black, 
                        alignCenter );

    
    if( showSaleFlag ) {
        
        drawSellStats( 0, 49, 0 );

        if( peekSaleFlag || revealSaleFlag ) {
            drawSellStats( 129, 49, 1 );


            if( peekSaleFlag && pictureSendSpriteSet ) {
                drawSprite( pictureSendSpriteID, 175, 99, white );
                }
            }
        

        int y = 39;
        
        if( selling[0] == 0 ) {
            font8->drawString( sellZeroNote,
                               128, y, white, alignCenter );
            }
        else {
            font8->drawString( sellSomeNote,
                               128, y, white, alignCenter );
            }
        
        }



    int activeUnit = getActiveUnit();
    
    int pictureSpriteID = -1;
    char pictureSpriteReady = false;

    if( activeUnit >= 0 && activeUnit < numUnits - 1 ) {

        if( activeUnit < numPlayerUnits ) {
            pictureSpriteID = pictureSprites[0][ activeUnit ];    
            }
        else {
            pictureSpriteID = pictureSprites[1][ activeUnit - numPlayerUnits ];
            }
    
    
        // use safe replacement to avoid graphical glitches on DS platform
        makeSpriteActive( pictureSpriteID, true );
        
    
        pictureSpriteReady = isSpriteReady( pictureSpriteID );
        }
    

    // don't show panel for inspector
    // delay showing PICTURE on panel until picture sprite loaded into RAM
    // BUT, draw rest of panel, to avoid panel flicker when switching instantly
    // from one active unit to another.
    if( activeUnit >= 0 && activeUnit < numUnits - 1  ) {
        
        Unit *u = getUnit( activeUnit );
        
        int pictureSpriteID;
        
        rgbaColor c;
        if( activeUnit < numPlayerUnits ) {
            c = panelColors[0];
            pictureSpriteID = pictureSprites[0][ activeUnit ];
            
            }
        else {
            c = panelColors[1];
            pictureSpriteID = pictureSprites[1][ activeUnit - numPlayerUnits ];
            }
        
        // let manilla show for now
        c = white;
        
        
        int panelTop = 34;
        


        //drawSprite( unitInfoPanelSprite, 0, panelTop, c );
        drawInfoPanelBackground( 0, panelTop, c );

        startNewSpriteLayer();
        

        // show stats for active unit

        drawUnit( activeUnit, 15, 22 + panelTop );

        drawDiamondCounter( 34, 14 + panelTop, u->mNumDiamondsHeld );
        
        // wait for it to load into RAM
        if( pictureSpriteReady ) {    
            drawSprite( pictureSpriteID, 11, 86, white );
            }
        

        int xOffset, x, y;
        
        if( u->mTripCost > 0 ) {
            
            xOffset = font8->measureString( translate( "stats_tripCost" ) );
            
            x = 54;
            y = 10 + panelTop;
            
            font8->drawString( translate( "stats_tripCost" ), 
                                x, 
                                y,
                                black, 
                                alignLeft );
            xOffset += 6;
            
            drawMoneyValue8( x + xOffset, y, 
                             u->mTripCost, 
                             black, false );
            }
        

        xOffset = font16Hand->measureString( translate( "stats_salary" ) );
        int otherOffset = 
            font16Hand->measureString( translate( "stats_bribes" ) );
        if( otherOffset > xOffset ) {
            xOffset = otherOffset;
            }
    
        xOffset += 10;
        
        x = 10;
        y = 27 + panelTop;
        
        font16Hand->drawString( translate( "stats_salary" ), 
                                x, 
                                y,
                                unitInfoPanelPaperColor, 
                                alignLeft );

        char hideSalary = false;
        if( activeUnit >= numPlayerUnits ) {
            // hide if not bribed
            if( u->mTotalBribe <= u->mTotalSalary ) {
                hideSalary = true;
                }
            }
        
        drawMoneyValue16( x + xOffset, y, 
                          u->mTotalSalary + u->mLastSalaryPayment, 
                          unitInfoPanelPaperColor, hideSalary );
        


        y += 20;
        
        font16Hand->drawString( translate( "stats_bribes" ), 
                            x, 
                            y,
                            unitInfoPanelPaperColor, 
                            alignLeft );

        char hideBribe = false;
        if( activeUnit < numPlayerUnits ) {
            // our unit
            
            // hide if bribing unit not bribed
            hideBribe = true;
            
            int lastBribingUnit = u->mLastBribingUnit;
            if( lastBribingUnit >= 0 ) {
                Unit *bribingUnit = getUnit( lastBribingUnit );
                
                if( bribingUnit->mTotalBribe > bribingUnit->mTotalSalary ) {
                    hideBribe = false;
                    }
                }

            if( ! u->mEnemyContactSinceBribeKnown ) {
                // old bribe knowledge still valid
                hideBribe = false;
                }
            }
        
        
        drawMoneyValue16( x + xOffset, y, 
                          u->mTotalBribe + u->mLastBribePayment, 
                          unitInfoPanelPaperColor, hideBribe );

        if( ! hideBribe && u->mLastBribingUnit >= 0 ) {            
            x = 242;
            
            drawUnit( u->mLastBribingUnit, x, 22 + panelTop );
            
            x -= 12;
            y = 7 + panelTop;

            font16Hand->drawString( translate( "stats_bribedBy" ), 
                                    x, 
                                    y,
                                    unitInfoPanelPaperColor, 
                                    alignRight );
            }
        

        char showBribedUnits = true;
        if( activeUnit >= numPlayerUnits ) {
            // enemy
            // hide if not bribed
            if( u->mTotalSalary >= u->mTotalBribe ) {
                showBribedUnits = false;
                }
            }
        
        
        if( showBribedUnits ) {


            int bribedCount = 0;
            
            for( int i=0; i<numPlayerUnits*2; i++ ) {
                if( getUnit(i)->mLastBribingUnit == activeUnit ) {
                    bribedCount ++;
                    }
                }

            if( bribedCount > 0 ) {
                
            
                font16Hand->drawString( translate( "stats_bribing" ), 
                                        230, 
                                        27 + panelTop,
                                        unitInfoPanelPaperColor, 
                                        alignRight );
                x = 193;
                y = 61 + panelTop;
                
                for( int i=0; i<numPlayerUnits*2; i++ ) {
                    Unit *other = getUnit(i);
                    
                    if( other->mLastBribingUnit == activeUnit ) {
                        
                        drawUnit( i, x, y );
                        
                        drawUnitBribe( i, x + 18, y - 9 );
                        
                        y += 22;
                        }
                    }

                }
            
            
            }
        
            
        }



    if( showInspectorPanelFlag ) {
        // show inspector destination panel
        Unit *u = getUnit( numUnits - 1 );

        int panelTop = 34;
        //drawSprite( unitInfoPanelSprite, 0, panelTop, white );
        drawInfoPanelBackground( 0, panelTop, white );

        startNewSpriteLayer();

        drawUnit( numUnits - 1, 15, 22 + panelTop );
        
        int dest = u->mDest;
        
        
        
        
        font16Hand->drawString( translate( "stats_inspectorBlocking" ), 
                                128,
                                11 + panelTop,
                                unitInfoPanelPaperColor, 
                                alignCenter );
        
        //int width = 
        //    font16->measureString( translate( "stats_inspectorBlocking" ) );
        

        drawDiamondCounter( 256 - 25, 
                            18 + panelTop, 
                            getDiamondsInRegion( dest ) );

        int i;
        char someFound = false;
        
        for( int i=0; i<numPlayerUnits*2; i++ ) {
            Unit *playerUnit = getUnit( i );
            if( playerUnit->mRegion == dest && 
                playerUnit->mNumDiamondsHeld > 0 ) {
                
                someFound = true;
                }
            }
        
        if( someFound ) {
            font16Hand->drawString( 
                translate( "stats_inspectorConfiscating" ), 
                128, 
                28 + panelTop,
                unitInfoPanelPaperColor, 
                alignCenter );
            
            
            int x = 96;
            int y = 61 + panelTop;
            
            for( i=0; i<numPlayerUnits*2; i++ ) {
                Unit *playerUnit = getUnit( i );
                if( playerUnit->mRegion == dest && 
                    playerUnit->mNumDiamondsHeld > 0 ) {

                    if( i < numPlayerUnits ) {
                        x = 96;
                        }
                    else {
                        x = 160;
                        }
                    
                    
                    drawUnit( i, x, y );

                    drawDiamondCounter( x+19, y-6, 
                                        playerUnit->mNumDiamondsHeld );

                    }
                }

            }
        
        
        }
    
    }

