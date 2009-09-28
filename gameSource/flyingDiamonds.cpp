#include "flyingDiamonds.h"
#include "colors.h"



static int miniDiamondSprite;
static int spriteW, spriteH;

typedef struct flyingDiamond {
        intPair mStart;
        intPair mEnd;
        int mNumSteps;
        int mStep;
    } flyingDiamond;

static flyingDiamond *currentDiamond = NULL;


void initFlyingDiamonds() {
    miniDiamondSprite = loadSprite( "miniDiamond.tga", 
                                    &spriteW, &spriteH,
                                    true );
    }

void freeFlyingDiamonds() {
    if( currentDiamond != NULL ) {
        delete currentDiamond;
        }
    }


void drawFlyingDiamonds() {
    if( currentDiamond != NULL ) {
        
        intPair start = currentDiamond->mStart;
        intPair end = currentDiamond->mEnd;
        
        int progress = currentDiamond->mStep;
        int numSteps = currentDiamond->mNumSteps;
        
        // blend pos
        intPair pos = end;
        pos.x *= progress;
        pos.y *= progress;
        
        pos.x += (numSteps - progress) * start.x;
        pos.y += (numSteps - progress) * start.y;
        
        pos.x /= numSteps;
        pos.y /= numSteps;
    

        drawSprite( miniDiamondSprite, 
                    pos.x - spriteW / 2, 
                    pos.y - spriteH / 2, trans75white );
        }
    }



void stepFlyingDiamonds() {
    if( currentDiamond != NULL ) {
        currentDiamond->mStep ++;
        
        if( currentDiamond->mStep >= currentDiamond->mNumSteps ) {
            delete currentDiamond;
            currentDiamond = NULL;
            }
        }
    }



void addFlyingDiamond( intPair inStart, intPair inEnd ) {
    if( currentDiamond == NULL ) {
        currentDiamond = new flyingDiamond;
        }
    currentDiamond->mStart = inStart;
    currentDiamond->mEnd = inEnd;
    currentDiamond->mStep = 0;
    
    // steps based on distance
    //int distance = intDistance( inStart, inEnd );
    
    //currentDiamond->mNumSteps = distance * 2;
    
    // same number of steps regardless of distance
    // (thus, for farther trips, diamonds move faster)
    currentDiamond->mNumSteps = 22;
    }



char flyingDiamondAnimationDone() {
    return (currentDiamond == NULL);
    }
