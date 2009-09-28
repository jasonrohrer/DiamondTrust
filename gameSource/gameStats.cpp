#include "gameStats.h"
#include "platform.h"
#include "common.h"
#include "Font.h"
#include "colors.h"
#include "units.h"

#include "minorGems/util/stringUtils.h"


static int money[2] = {0,0};
static int diamonds[2] = {0,0};
static int selling[2] = {0,0};



static char showSaleFlag = false;
static char peekSaleFlag = false;
static char revealSaleFlag = false;

static char *sellZeroNote = NULL;
static char *sellSomeNote = NULL;



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


static int statusPanelSprite;
static int panelW, panelH;

static int unitInfoPanelSprite;
static int unitPanelW, unitPanelH;

void initStats() {
    statusPanelSprite = loadSprite( "statsPanel.tga", 
                                    &panelW, &panelH, true );
    unitInfoPanelSprite = loadSprite( "unitInfoPanel.tga", 
                                      &unitPanelW, &unitPanelH, true );

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





static rgbaColor panelColors[2] = { playerRegionColor, enemyRegionColor };


extern Font *font16;
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
                        inY - 8,
                        diamondPurple, 
                        alignCenter );

    delete [] countString;
    }



static void drawPanelContents( int inX, int inPlayer ) {
    
    int baseY = 0;
    

    font16->drawString( "$", inX + 10, baseY + 6, black, alignLeft );

    char *moneyString = autoSprintf( "%d", money[ inPlayer ] );

    font16->drawString( moneyString, 
                        inX + 48, 
                        baseY + 8,
                        black, 
                        alignRight );

    delete [] moneyString;
    

    drawDiamondCounter( inX + 90, baseY + panelH / 2, diamonds[ inPlayer ] );
    
    }




static void drawMoneyValue( int inX, int inY, int inValue, 
                            rgbaColor inColor, char inValueHidden ) {

    font16->drawString( "$", inX, inY - 2, inColor, alignLeft );
    
    char *moneyString;
        
    if( inValueHidden ) {
        moneyString = autoSprintf( "?" );
        }
    else {
        moneyString = autoSprintf( "%d", inValue );
        }
    
    
    font16->drawString( moneyString, 
                        inX + 30, 
                        inY,
                        inColor, 
                        alignRight );

    delete [] moneyString;
    }



static void drawSellStats( int inX, int inPlayer ) {
    int y = 50;
    
    int xOffset = font16->measureString( translate( "stats_selling" ) );
    int otherOffset = 
        font16->measureString( translate( "stats_earning" ) );
    if( otherOffset > xOffset ) {
        xOffset = otherOffset;
        }
    
    xOffset += 20;
    
    font16->drawString( translate( "stats_selling" ), 
                        inX, 
                        y - 8,
                        white, 
                        alignLeft );
    
    
    drawDiamondCounter( inX + xOffset, y, selling[inPlayer] );

    y += 20;
    font16->drawString( translate( "stats_earning" ), 
                        inX, 
                        y,
                        white, 
                        alignLeft );
    
    
    
    if( selling[inPlayer] == 0 ) {
        drawMoneyValue( inX + xOffset - 8, y, noSaleFlatRate, white, false );
        }
    else {

        drawMoneyValue( inX + xOffset - 8, y, 
                        getPlayerEarnings( inPlayer ), white, 
                        !revealSaleFlag );
        }
    }



// draws stats onto upper screen
void drawStats() {
    drawSprite( statusPanelSprite, 0, 0, panelColors[0] );

    drawSprite( statusPanelSprite, panelW, 0, panelColors[1] );

    startNewSpriteLayer();

    drawPanelContents( 0, 0 );
    drawPanelContents( panelW, 1 );


    if( showSaleFlag ) {
        
        drawSellStats( 10, 0 );

        if( peekSaleFlag || revealSaleFlag ) {
            drawSellStats( 138, 1 );
            }
        

        int y = 90;
        
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
    
    // don't show panel for inspector
    if( activeUnit >= 0 && activeUnit < numUnits - 1 ) {
        
        Unit *u = getUnit( activeUnit );
        
        rgbaColor c;
        if( activeUnit < numPlayerUnits ) {
            c = panelColors[0];
            }
        else {
            c = panelColors[1];
            }
        
        drawSprite( unitInfoPanelSprite, 64, 64, c );

        startNewSpriteLayer();
        

        // show stats for active unit

        drawUnit( activeUnit, 74, 84 );

        drawDiamondCounter( 79, 94, u->mNumDiamondsHeld );
        

        int xOffset = font16->measureString( translate( "stats_salary" ) );
        int otherOffset = 
            font16->measureString( translate( "stats_bribes" ) );
        if( otherOffset > xOffset ) {
            xOffset = otherOffset;
            }
    
        xOffset += 20;
        
        int x = 84;
        int y = 94;
        
        font16->drawString( translate( "stats_salary" ), 
                            x, 
                            y,
                            black, 
                            alignLeft );

        char hideSalary = false;
        if( activeUnit >= numPlayerUnits ) {
            // hide if not bribed
            if( u->mTotalBribe <= u->mTotalSalary ) {
                hideSalary = true;
                }
            }
        
        font16->drawString( translate( "stats_salary" ), 
                            x, 
                            y,
                            black, 
                            alignLeft );
        drawMoneyValue( x + xOffset, y, u->mTotalSalary, black, hideSalary );
        
        }
    
    }

