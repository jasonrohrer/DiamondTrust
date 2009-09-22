#include "gameStats.h"
#include "platform.h"
#include "common.h"
#include "Font.h"
#include "colors.h"

#include "minorGems/util/stringUtils.h"


static int money[2] = {0,0};
static int diamonds[2] = {0,0};




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

void initStats() {
    statusPanelSprite = loadSprite( "statsPanel.tga", 
                                    &panelW, &panelH, false );
    }


void freeStats() {
    }




static rgbaColor panelColors[2] = { playerRegionColor, enemyRegionColor };


extern Font *font16;


// from map.cpp
extern int diamondSpriteID;
extern int diamondSpriteW;
extern int diamondSpriteH;



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
    

    intPair diamondCountPos = { inX + 90, baseY + panelH / 2 };
    
    
    drawSprite( diamondSpriteID, 
                diamondCountPos.x - diamondSpriteW/2, 
                diamondCountPos.y - diamondSpriteH/2, 
                white );

    startNewSpriteLayer();
    
    char *countString = autoSprintf( "%d", diamonds[ inPlayer ] );
    
    font16->drawString( countString, 
                        diamondCountPos.x, 
                        diamondCountPos.y - 8,
                        diamondPurple, 
                        alignCenter );

    delete [] countString;
    
    }

        

// draws stats onto upper screen
void drawStats() {
    drawSprite( statusPanelSprite, 0, 0, panelColors[0] );

    drawSprite( statusPanelSprite, panelW, 0, panelColors[1] );

    startNewSpriteLayer();

    drawPanelContents( 0, 0 );
    drawPanelContents( panelW, 1 );
    }

