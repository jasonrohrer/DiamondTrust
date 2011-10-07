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

static int stepsPerHandPostion = 11;


static int currentHourHand = 0;
static int currentHourHandDisplaySteps = 0;
// everytime minute hand rotates 1.5 times, hour hand should advance to next
// 8th of the clock
static int stepsPerHourHandPosition = stepsPerHandPostion * 12;


static char lastStepTimeSet = false;
static unsigned int lastStepTimeMS;




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



void resetWatch() {
    currentWatchHand = 0;

    currentWatchHandDisplaySteps = 0;
    
    
    currentHourHand = 0;
    
    currentHourHandDisplaySteps = 0;
    
    lastStepTimeSet = false;
    }



void stepWatch() {

    if( !lastStepTimeSet ) {
        lastStepTimeMS = getSystemMilliseconds();
    
        lastStepTimeSet = true;
        
        return;
        }
    
    
    // normalize step so that watch advances at same realtime speed 
    // no matter how much time passes between steps (like when AI is
    // in fully CPU mode an steps are slow)
    
    // normalize to 33 ms per step (30 fps)

    unsigned int thisStepTimeMS = getSystemMilliseconds();
    

    unsigned int dMS = thisStepTimeMS - lastStepTimeMS;
    

    // take multiple watch steps now to catch up, if needed
    int numStepsToTake = dMS / 33;
    

    // always take at least one step
    if( numStepsToTake < 1 ) {
        numStepsToTake = 1;
        }
    
    lastStepTimeMS = thisStepTimeMS;
    


    for( int s=0; s<numStepsToTake; s++ ) {

        currentWatchHandDisplaySteps ++;
    

        if( currentWatchHandDisplaySteps >= stepsPerHandPostion ) {
            currentWatchHandDisplaySteps = 0;
        
            currentWatchHand ++;
        
            if( currentWatchHand > 7 ) {
                // full rotation

                currentWatchHand = 0;
                }
            }

        currentHourHandDisplaySteps ++;
    
        if( currentHourHandDisplaySteps >= stepsPerHourHandPosition ) {
            currentHourHandDisplaySteps = 0;
        
            currentHourHand ++;
        
            if( currentHourHand > 7 ) {
                currentHourHand = 0;
                }
            }
        }
    }



// draws it in a fixed position on top screen
void drawWatch() {
    
    // move down to center it between Sell phase ledger paper and greenbar
    int yOffset = 27;
    


    drawSprite( watchSpriteID, 
                112, 
                80 + yOffset, 
                white );
    
    startNewSpriteLayer();

    rgbaColor minuteHandColor = { 255, 255, 255, 96 };
    
    drawSprite( watchHandSpriteID[ currentWatchHand ],
                watchHandPositions[ currentWatchHand ].x,
                watchHandPositions[ currentWatchHand ].y + yOffset,
                minuteHandColor );                

    drawSprite( watchHandSpriteID[ currentHourHand ],
                watchHandPositions[ currentHourHand ].x,
                watchHandPositions[ currentHourHand ].y + yOffset,
                white );                
    }
