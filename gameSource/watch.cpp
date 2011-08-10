#include "watch.h"

#include "platform.h"
#include "colors.h"
#include "common.h"



static int watchSpriteID;

static int watchHandSpriteID[8];

static intPair watchHandPositions[8] = { { 123, 90 },
                                         { 125, 91 },
                                         { 126, 93 },
                                         { 126, 95 },
                                         { 123, 95 },
                                         { 121, 95 },
                                         { 121, 93 },
                                         { 122, 91 } };


static int currentWatchHand = 0;

static int currentWatchHandDisplaySteps = 0;

static int stepsPerHandPostion = 5;


static int currentHourHand = 0;
static int currentHourHandSubRotations = 0;
static int rotationsPerHourHandPosition = 1;




void initWatch() {
    watchSpriteID = loadSprite( "watch.tga", true );    
    
    
    watchHandSpriteID[0] = loadSprite( "watchHandVertical.tga", true );    
    watchHandSpriteID[1] = loadSprite( "watchHandDiagonalRight.tga", true );
    watchHandSpriteID[2] = loadSprite( "watchHandHorizontal.tga", true );
    watchHandSpriteID[3] = loadSprite( "watchHandDiagonalLeft.tga", true );

    // repeat for other half
    watchHandSpriteID[4] = watchHandSpriteID[0];
    watchHandSpriteID[5] = watchHandSpriteID[1];
    watchHandSpriteID[6] = watchHandSpriteID[2];
    watchHandSpriteID[7] = watchHandSpriteID[3];
    
    }


void freeWatch() {
    }



void stepWatch() {
    currentWatchHandDisplaySteps ++;
    

    if( currentWatchHandDisplaySteps >= stepsPerHandPostion ) {
        currentWatchHandDisplaySteps = 0;
        
        currentWatchHand ++;
        
        if( currentWatchHand > 7 ) {
            // full rotation

            currentWatchHand = 0;
            
            currentHourHandSubRotations ++;
            
            if( currentHourHandSubRotations >= rotationsPerHourHandPosition ) {
                // step the hour hand

                currentHourHandSubRotations = 0;
                currentHourHand ++;
                
                if( currentHourHand > 7 ) {
                    currentHourHand = 0;
                    }
                }
            
            }
        }
    }



// draws it in a fixed position on top screen
void drawWatch() {
    
    
    drawSprite( watchSpriteID, 
                112, 
                80, 
                white );
    
    startNewSpriteLayer();
    
    drawSprite( watchHandSpriteID[ currentWatchHand ],
                watchHandPositions[ currentWatchHand ].x,
                watchHandPositions[ currentWatchHand ].y,
                white );                

    drawSprite( watchHandSpriteID[ currentHourHand ],
                watchHandPositions[ currentHourHand ].x,
                watchHandPositions[ currentHourHand ].y,
                white );                
    }
